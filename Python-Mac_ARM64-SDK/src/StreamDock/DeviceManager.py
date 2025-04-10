import threading
import time
import hid
from .ProductIDs import g_products
from .Transport.LibUSBHIDAPI import LibUSBHIDAPI


class DeviceManager:
    streamdocks = list()

    @staticmethod
    def _get_transport(transport):
        return LibUSBHIDAPI()

    def __init__(self, transport=None):
        self.transport = self._get_transport(transport)
        self._poll_thread = None
        self._running = False

    def enumerate(self):
        products = g_products
        for vid, pid, class_type in products:
            found_devices = self.transport.enumerate(vid=vid, pid=pid)
            self.streamdocks.extend([
                class_type(self.transport, d) for d in found_devices
            ])
        return self.streamdocks

    def listen(self, poll_interval=1.0):
        """
        使用 hid.enumerate() 在 macOS 上轮询检测 USB 插拔事件
        """
        self._running = True
        vid_pid_list = [(vid, pid, cls) for vid, pid, cls in g_products]
        last_seen = set()

        def poll_loop():
            nonlocal last_seen
            while self._running:
                devices = hid.enumerate()
                current = set()
                path_map = {}
                for d in devices:
                    vid, pid, usage = d['vendor_id'], d['product_id'], d['usage']
                    for p_vid, p_pid, class_type in vid_pid_list:
                        if vid == p_vid and pid == p_pid and usage == 1:
                            path = d['path'].decode() if isinstance(d['path'], bytes) else d['path']
                            current.add(path)
                            path_map[path] = (vid, pid, class_type)

                added = current - last_seen
                removed = last_seen - current

                for path in added:
                    vid, pid, cls = path_map[path]
                    print("[add] path:", path)
                    found_devices = self.transport.enumerate(vid, pid)
                    for d in found_devices:
                        if d['path'].endswith(path):
                            new_dev = cls(self.transport, d)
                            self.streamdocks.append(new_dev)
                            new_dev.open()
                            break

                for path in removed:
                    for dev in self.streamdocks:
                        if dev.getPath() in path:
                            print("[remove] path:", dev.getPath())
                            self.streamdocks.remove(dev)
                            # your reconnect logic like the next two line
                            # newDevice.set_key_image(1, "../img/tiga64.png")
                            # newDevice.refresh()
                            break

                last_seen = current
                time.sleep(poll_interval)

        self._poll_thread = threading.Thread(target=poll_loop, daemon=True)
        self._poll_thread.start()

    def stop(self):
        """
        停止轮询监听（用于关闭监听线程）
        """
        self._running = False
