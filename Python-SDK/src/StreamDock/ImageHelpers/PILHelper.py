import io
from PIL import Image


def _create_image(image_format, background):
    return Image.new("RGB", image_format['size'], background)


def _scale_image(image, image_format, margins=[0, 0, 0, 0], background='black'):
    if len(margins) != 4:
        raise ValueError("Margins should be given as an array of four integers.")

    final_image = _create_image(image_format, background=background)

    thumbnail_max_width = final_image.width - (margins[1] + margins[3])
    thumbnail_max_height = final_image.height - (margins[0] + margins[2])

    thumbnail = image.convert("RGBA")
    thumbnail.thumbnail((thumbnail_max_width, thumbnail_max_height), Image.LANCZOS)

    thumbnail_x = (margins[3] + (thumbnail_max_width - thumbnail.width) // 2)
    thumbnail_y = (margins[0] + (thumbnail_max_height - thumbnail.height) // 2)

    final_image.paste(thumbnail, (thumbnail_x, thumbnail_y), thumbnail)

    return final_image


def _to_rgb_with_background(image, background='black'):
    rgba_image = image.convert("RGBA")
    background_image = Image.new("RGBA", rgba_image.size, background)
    return Image.alpha_composite(background_image, rgba_image).convert("RGB")


def _to_native_format(image, image_format):
    image_type = image_format["format"].lower()
    if image_type not in ("jpeg", "jpg", "png", "raw"):
        raise ValueError(
            f"Unsupported image format: {image_format['format']}"
        )
    
    if image.size != image_format['size']:
        image = image.resize(image_format["size"], Image.Resampling.BOX)

    # Pillow's positive angles are counterclockwise. Dimensions describe the input canvas.
    if image_format['rotation']:
        image = image.rotate(image_format['rotation'], expand=True)

    if image_format['flip'][0]:
        image = image.transpose(Image.FLIP_LEFT_RIGHT)

    if image_format['flip'][1]:
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
    
    if image_type == "png":
        image = image.convert("RGBA")
    else:
        image = _to_rgb_with_background(image)
    
    return image


def _has_alpha(image):
    return "A" in image.getbands() or "transparency" in image.info


def _argb4444(image):
    pixels = bytearray(image.convert("RGBA").tobytes())
    for offset in range(0, len(pixels), 4):
        alpha = pixels[offset + 3]
        alpha4 = alpha >> 4
        for channel in range(3):
            value4 = ((pixels[offset + channel] * alpha + 127) // 255) >> 4
            pixels[offset + channel] = min(255, value4 * 255 // alpha4) if alpha4 else 0
        pixels[offset + 3] = alpha4 * 17
    return Image.frombytes("RGBA", image.size, bytes(pixels))


def _encode(image, image_type, quality):
    buffer = io.BytesIO()
    if image_type == "PNG":
        compression = 6 if quality < 0 else (100 - quality) * 9 // 91
        image.save(buffer, "PNG", compress_level=compression)
    else:
        image.save(buffer, "JPEG", quality=75 if quality < 0 else quality)
    return buffer.getvalue()


def encode_key_image(image, image_format, animated=False, xl=False):
    """Encode key images using JPEG/PNG byte targets and a quantized fallback."""
    native_format = dict(image_format)
    auto_png = native_format["format"].upper() == "PNG"
    image_type = "PNG" if auto_png and _has_alpha(image) else "JPEG"
    native_format["format"] = image_type
    image = _to_native_format(image, native_format)
    quality = 100
    target = (2040 if xl else 3000) if animated else 9000
    if image_type == "PNG":
        quality = 50 if animated else -1
        target = 20000 if animated else 29000
        if animated:
            image = _argb4444(image)
    data = _encode(image, image_type, quality)
    while len(data) > target:
        quality -= 2
        if quality <= 0:
            image = _argb4444(image)
            if image_type == "JPEG":
                image = _to_rgb_with_background(image)
            return _encode(image, image_type, -1)
        data = _encode(image, image_type, quality)
    return data


def encode_boot_logo(image, image_format, n1=False):
    """Resize boot logos to the model canvas, then rotate and encode their pixels."""
    image = _to_native_format(image, image_format)
    if image_format["format"].upper() == "RAW":
        if image_format.get("pixel_format", "BGR888") == "BGR888":
            return image.tobytes("raw", "BGR")
        pixels = image.tobytes()
        data = bytearray()
        for offset in range(0, len(pixels), 3):
            r, g, b = pixels[offset:offset + 3]
            value = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
            data.extend(value.to_bytes(2, "little"))
        return bytes(data)
    for quality in range(100, -1, -10):
        data = _encode(image, "JPEG", quality)
        if not n1 or len(data) <= 302080:
            break
    if len(data) > 522240:
        raise ValueError("Boot logo exceeds 522240 bytes")
    return data


def encode_background_frame(image, image_format):
    native_format = dict(image_format, format="JPEG")
    image = _to_native_format(image, native_format)
    return _encode(image, "JPEG", 60)


def create_image(dock, background='black'):
    return create_key_image(dock, background)


def create_key_image(dock, background='black'):
    return _create_image(dock.key_image_format(), background)


def create_touchscreen_image(dock, background='black'):
    return _create_image(dock.touchscreen_image_format(), background)


def create_scaled_image(dock, image, margins=[0, 0, 0, 0], background='black'):
    return create_scaled_key_image(dock, image, margins, background)


def create_scaled_key_image(dock, image, margins=[0, 0, 0, 0], background='black'):
    return _scale_image(image, dock.key_image_format(), margins, background)


def create_scaled_touchscreen_image(dock, image, margins=[0, 0, 0, 0], background='black'):
    return _scale_image(image, dock.touchscreen_image_format(), margins, background)

def to_native_key_format(dock, image, key=None):
    image_format = dock.key_image_format() if key is None else dock.key_image_format(key)
    return _to_native_format(image, image_format)

def to_native_seondscreen_format(dock, image):
    return _to_native_format(image, dock.secondscreen_image_format())

def to_native_touchscreen_format(dock, image):
    return _to_native_format(image, dock.touchscreen_image_format())
