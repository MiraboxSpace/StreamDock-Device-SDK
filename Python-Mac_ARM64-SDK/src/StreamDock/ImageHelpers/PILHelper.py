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


def _to_native_format(image, image_format):
    if image_format["format"].lower() != "jpeg" and image_format["format"].lower() != "jpg":
        raise ValueError(f"no support format: {image_format['format']}. only 'jpeg' or 'jpg' is supported")
    
    _expand = True
    if image.size[1] == image_format["size"][0] and image.size[0] == image_format["size"][1]:
        _expand = False
    
    # must rotate the picture first then resize the picture
    if image_format["rotation"] == 90 or image_format["rotation"] == -90:
        swapped_tuple = (image_format["size"][1], image_format["size"][0])
        image_format["size"] = swapped_tuple
    
    if image_format['rotation']:
        image = image.rotate(image_format['rotation'], expand = _expand)
    
    if image.size != image_format['size']:
        image = image.resize(image_format["size"])

    if image_format['flip'][0]:
        image = image.transpose(Image.FLIP_LEFT_RIGHT)

    if image_format['flip'][1]:
        image = image.transpose(Image.FLIP_TOP_BOTTOM)
    
    image = image.convert('RGB')
    
    return image


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

def to_native_key_format(dock, image):
    return _to_native_format(image, dock.key_image_format())

def to_native_seondscreen_format(dock, image):
    return _to_native_format(image, dock.secondscreen_image_format())

def to_native_touchscreen_format(dock, image):
    return _to_native_format(image, dock.touchscreen_image_format())
