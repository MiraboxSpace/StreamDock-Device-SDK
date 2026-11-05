// =============================================================================
// StreamDock WebSocket SDK - Complete Test Suite
// =============================================================================
// This file demonstrates all available WebSocket commands for StreamDock devices
//
// Requirements:
// - Node.js environment
// - WebSocket server running on ws://127.0.0.1:9002
// - ws package: npm install ws
// - Test images in img/ directory (resolved relative to project root)
// - H1 Pro GIF upload: ffmpeg executable available on the server's PATH
// - Optional H1PRO_TEST_MP4_PATH: MP4 container, MJPEG video, 240x320 portrait,
//   no audio, at most 5 MiB, to test direct upload
//   Uploading restarts and re-enumerates H1 Pro; use its new device path.
//
// Usage:
// - Uncomment the test section you want to run
// - Run with: node docs/test.js
// =============================================================================

const WebSocket = require('ws');
const path = require('path');
const fs = require('fs');
const { spawnSync } = require('child_process');

// =============================================================================
// CONFIGURATION
// =============================================================================

const WS_URL = 'ws://127.0.0.1:9002';
const LOCAL_IMG_DIR = path.join(__dirname, 'img');
const PYTHON_SDK_IMG_DIR = path.resolve(__dirname, '..', '..', 'Python-SDK', 'img');

const TEST_IMAGE_PATH = resolveAsset('button_test.jpg');
const TEST_BACKGROUND_PATH = resolveAsset('backgroud_test.png');
const TEST_FRAME_BACKGROUND_PATH = resolveAsset('backgroud_test2.png');
const TEST_PNG_KEY_IMAGE_PATH = resolveAsset('mark.png');
const TEST_KEY_GIF_PATH = resolveAsset('test.gif');
const TEST_BACKGROUND_GIF_PATH = resolveAsset('backgroud_test.gif');

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Base64 encode a string
 */
function base64Encode(str) {
  return Buffer.from(str).toString('base64');
}

function resolveAsset(fileName) {
  const localPath = path.join(LOCAL_IMG_DIR, fileName);
  if (fs.existsSync(localPath)) {
    return localPath;
  }

  const pythonSdkPath = path.join(PYTHON_SDK_IMG_DIR, fileName);
  if (fs.existsSync(pythonSdkPath)) {
    return pythonSdkPath;
  }

  return localPath;
}

/**
 * Send a command to the WebSocket server
 */
function sendCommand(ws, event, path, payload = {}) {
  const command = {
    event: event,
    path: path,
    payload: payload
  };
  ws.send(JSON.stringify(command));
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

function uploadH1ProAndWaitForReconnect(ws, event, devicePath, deviceInfo, payload) {
  return new Promise((resolve, reject) => {
    let uploadError = '';
    let errorTimer;
    const timer = setTimeout(() => {
      cleanup();
      reject(new Error(`H1 Pro did not re-enumerate after ${event} within 8 minutes${uploadError ? `; server error: ${uploadError}` : ''}`));
    }, 8 * 60 * 1000);

    function cleanup() {
      clearTimeout(timer);
      clearTimeout(errorTimer);
      ws.removeListener('message', onMessage);
      ws.removeListener('close', onClose);
    }

    function onClose() {
      cleanup();
      reject(new Error('WebSocket closed while waiting for H1 Pro re-enumeration'));
    }

    function onMessage(data) {
      const msg = JSON.parse(data);
      if (msg.event === 'error' && msg.path === devicePath &&
          typeof msg.payload?.message === 'string' && msg.payload.message.startsWith(`${event}:`)) {
        uploadError = msg.payload.message;
        console.warn(`[H1Pro] Upload reported an error; waiting for possible firmware restart: ${uploadError}`);
        if (!errorTimer) {
          errorTimer = setTimeout(() => {
            cleanup();
            reject(new Error(uploadError));
          }, 30000);
        }
      }
      if (msg.event === 'deviceDidDisconnect' && msg.path === devicePath) {
        console.log('[H1Pro] Old USB handle disconnected');
      }
      if (msg.event !== 'deviceDidConnect' || msg.payload?.Type !== 'H1Pro') {
        return;
      }
      if (deviceInfo.SerialNumber && msg.payload.SerialNumber !== deviceInfo.SerialNumber) {
        return;
      }
      if (!deviceInfo.SerialNumber && msg.payload.ProductID !== deviceInfo.ProductID) {
        return;
      }
      cleanup();
      console.log(`[H1Pro] Re-enumerated; new device path: ${msg.path}`);
      resolve(msg.path);
    }

    ws.on('message', onMessage);
    ws.once('close', onClose);
    sendCommand(ws, event, devicePath, payload);
  });
}

function switchH1ProModeAndWait(ws, devicePath, mode) {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => {
      cleanup();
      reject(new Error(`H1 Pro ${mode} mode command timed out`));
    }, 10000);

    function cleanup() {
      clearTimeout(timer);
      ws.removeListener('message', onMessage);
      ws.removeListener('close', onClose);
    }

    function onClose() {
      cleanup();
      reject(new Error(`WebSocket closed while switching H1 Pro to ${mode} mode`));
    }

    function onMessage(data) {
      const msg = JSON.parse(data);
      if (msg.path !== devicePath) {
        return;
      }
      if (msg.event === 'error' && msg.payload?.message?.startsWith('switchH1ProMode:')) {
        cleanup();
        reject(new Error(msg.payload.message));
      } else if (msg.event === 'switchH1ProMode' && msg.payload?.mode === mode) {
        cleanup();
        resolve();
      }
    }

    ws.on('message', onMessage);
    ws.once('close', onClose);
    sendCommand(ws, 'switchH1ProMode', devicePath, { mode });
  });
}

function keyCountForDevice(deviceType) {
  const keyCounts = {
    '293': 15,
    '293V3': 15,
    '293s': 18,
    '293sV3': 18,
    'N3': 6,
    'N3EN': 6,
    'N4': 14,
    'N4EN': 14,
    'N1': 18,
    'N1EN': 18,
    'N4Pro': 14,
    'XL': 32,
    'M18': 15,
    'M3': 15,
    'K1Pro': 6,
    'H1Pro': 12,
    'Mini': 6
  };
  return keyCounts[deviceType] || 18;
}

function imageKeysForDevice(deviceType) {
  // N4 Pro uses slots 1-4 and 6-15; slot 5 is reserved.
  if (deviceType === 'N4Pro') {
    return [1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15];
  }
  const keyCount = keyCountForDevice(deviceType);
  return Array.from({ length: keyCount }, (_, index) => index + 1);
}

/**
 * Wait for WebSocket connection to be established
 */
function waitForConnection(ws, timeout = 5000) {
  return new Promise((resolve, reject) => {
    if (ws.readyState === WebSocket.OPEN) {
      resolve();
      return;
    }

    const timer = setTimeout(() => {
      reject(new Error('Connection timeout'));
    }, timeout);

    ws.once('open', () => {
      clearTimeout(timer);
      resolve();
    });
  });
}

/**
 * Wait for device path from deviceDidConnect event
 */
function waitForDevicePath(ws, timeout = 5000) {
  return new Promise((resolve, reject) => {
    const timer = setTimeout(() => {
      reject(new Error('Device connection timeout'));
    }, timeout);

    const messageHandler = (data) => {
      const msg = JSON.parse(data);
      if (msg.event === 'deviceDidConnect') {
        clearTimeout(timer);
        ws.removeListener('message', messageHandler);
        console.log(`Device connected: ${msg.payload.Product}`);
        console.log(`Device type: ${msg.payload.Type}`);
        console.log(`Firmware: ${msg.payload.FirmwareVersion}`);
        console.log(`Serial: ${msg.payload.SerialNumber}`);
        resolve(msg.path);
      }
    };

    ws.on('message', messageHandler);
  });
}

/**
 * Create a WebSocket connection and wait for device
 */
async function createConnectionAndListen() {
  return new Promise((resolve, reject) => {
    const ws = new WebSocket(WS_URL);
    let devicePath = null;

    ws.on('open', () => {
      console.log('WebSocket connected');
    });

    ws.on('message', (data) => {
      const msg = JSON.parse(data);
      if (msg.event === 'deviceDidConnect') {
        devicePath = msg.path;
        console.log(`Device connected: ${msg.payload.Product}`);
        resolve({ ws, devicePath, deviceInfo: msg.payload });
      }

      if (msg.event === 'error') {
        console.error('Error:', msg.payload.message);
      }
    });

    ws.on('error', (error) => {
      console.error('WebSocket error:', error);
      reject(error);
    });

    ws.on('close', () => {
      console.log('WebSocket closed');
    });

    // Timeout after 5 seconds
    setTimeout(() => {
      if (!devicePath) {
        reject(new Error('No device connected within 5 seconds'));
      }
    }, 5000);
  });
}


async function testComprehensive() {
  console.log('\n=== COMPREHENSIVE TEST (Like Python SDK main.py) ===');

  try {
    const connection = await createConnectionAndListen();
    const { ws, deviceInfo } = connection;
    let devicePath = connection.devicePath;

    console.log(`\nDevice Info:`);
    console.log(`  Path: ${deviceInfo.Path}`);
    console.log(`  Type: ${deviceInfo.Type}`);
    console.log(`  Firmware: ${deviceInfo.FirmwareVersion}`);
    console.log(`  Serial: ${deviceInfo.SerialNumber}`);

    // setBackgroundImg / setBackgroundImgData events set the static boot logo in ROM. Keep this disabled to match Python SDK main.py.
    // Do not use it as a dynamic background or repeatedly write animation frames.
    // Keep the model-required resolution and orientation; changing them may brick the device. We recommend using the official StreamDock software to set the boot logo.
    // sendCommand(ws, 'setBackgroundImg', devicePath, {
    //   imagePath: TEST_BACKGROUND_PATH
    // });
    sendCommand(ws, 'refresh', devicePath, {});
    await sleep(2000);

    // Device-specific tests
    const deviceType = deviceInfo.Type;
    const firmware = deviceInfo.FirmwareVersion || '';
    const keyGifSupported = !(
      ['293', '293s'].includes(deviceType) ||
      (['N3', 'N3EN'].includes(deviceType) &&
        (deviceInfo.VendorID === 0x6602 || firmware.includes('V25.N3'))) ||
      (deviceType === 'M18' && firmware.includes('V2.M18'))
    );

    // N4Pro special functions
    if (deviceType === 'N4Pro') {
      console.log('\n--- N4Pro special functions ---');
      sendCommand(ws, 'setLEDBrightness', devicePath, { brightness: 255 });
      // sendCommand(ws, 'setLEDColor', devicePath, { r: 0, g: 0, b: 255 });
      // N4 Pro supports control single LED colors
      sendCommand(ws, 'setSingleLedColor', devicePath, {
        colors: [
          [255, 0, 0],
          [0, 0, 255],
          [255, 0, 255],
          [255, 255, 0]
        ]
      });
      sendCommand(ws, 'setTemporaryBackgroundImg', devicePath, {
        imagePath: TEST_FRAME_BACKGROUND_PATH
      });
      sendCommand(ws, 'setDeviceConfig', devicePath, {
        enableVibration: false
      });
      // Python SDK keeps N4Pro background GIF commented out. Uncomment if needed:
      // sendCommand(ws, 'setBackgroundGif', devicePath, { imagePath: TEST_BACKGROUND_GIF_PATH });
      await sleep(2000);
    }

    // XL special functions
    if (deviceType === 'XL') {
      console.log('\n--- XL special functions ---');
      sendCommand(ws, 'setBackgroundGif', devicePath, {
        imagePath: TEST_BACKGROUND_GIF_PATH
      });
      sendCommand(ws, 'setDeviceConfig', devicePath, {
        ledFollowKeyLight: true
      });
      await sleep(2000);
    }

    // K1Pro special functions
    if (deviceType === 'K1Pro') {
      console.log('\n--- K1Pro special functions ---');
      sendCommand(ws, 'setKeyboardBacklightBrightness', devicePath, { brightness: 6 });
      sendCommand(ws, 'setKeyboardLightingSpeed', devicePath, { speed: 3 });
      sendCommand(ws, 'setKeyboardLightingEffects', devicePath, { effect: 0 });
      sendCommand(ws, 'setKeyboardRGBBacklight', devicePath, { r: 255, g: 0, b: 0 });
      sendCommand(ws, 'setKeyboardOSMode', devicePath, { osMode: 0 });
    }

    // H1 Pro / H1 ProE special functions
    if (deviceType === 'H1Pro') {
      console.log('\n--- H1Pro special functions ---');
      console.log('[H1Pro] Stage 1: check ffmpeg on PATH and upload the GIF');
      const ffmpeg = spawnSync('ffmpeg', ['-version'], { stdio: 'ignore' });
      if (ffmpeg.error || ffmpeg.status !== 0) {
        console.warn('[H1Pro] ffmpeg is unavailable on PATH; skipping GIF upload');
      } else if (!fs.existsSync(TEST_BACKGROUND_GIF_PATH)) {
        console.warn(`[H1Pro] GIF asset is missing: ${TEST_BACKGROUND_GIF_PATH}`);
      } else {
        console.log('[H1Pro] Uploading GIF to device storage; firmware will restart');
        devicePath = await uploadH1ProAndWaitForReconnect(
          ws, 'uploadH1ProGif', devicePath, deviceInfo,
          { imagePath: TEST_BACKGROUND_GIF_PATH }
        );
      }

      const mp4Path = process.env.H1PRO_TEST_MP4_PATH;
      if (mp4Path) {
        console.log('[H1Pro] Stage 2: upload the prepared MJPEG MP4');
        if (!fs.existsSync(mp4Path)) {
          throw new Error(`H1PRO_TEST_MP4_PATH does not exist: ${mp4Path}`);
        }
        devicePath = await uploadH1ProAndWaitForReconnect(
          ws, 'uploadH1ProMp4', devicePath, deviceInfo, { videoPath: mp4Path }
        );
      } else {
        console.log('[H1Pro] Stage 2: MP4 upload skipped (set H1PRO_TEST_MP4_PATH to test it)');
      }

      console.log('[H1Pro] Stage 3: refresh the re-enumerated display and wait 2 seconds');
      sendCommand(ws, 'refresh', devicePath, {});
      await sleep(2000);
      console.log('[H1Pro] Stage 4: switch to GIF mode');
      await switchH1ProModeAndWait(ws, devicePath, 'gif');
      console.log('[H1Pro] GIF mode command accepted');
      await sleep(5000);
      console.log('[H1Pro] Stage 5: switch to SCREENSAVER mode');
      await switchH1ProModeAndWait(ws, devicePath, 'screensaver');
      await sleep(5000);
      console.log('[H1Pro] Stage 6: switch to KEYS mode');
      await switchH1ProModeAndWait(ws, devicePath, 'keys');
      sendCommand(ws, 'clearAllIcon', devicePath, {});
      sendCommand(ws, 'refresh', devicePath, {});
      await sleep(5000);
      console.log('[H1Pro] Stage 7: set brightness and 12 key images/GIFs');
      sendCommand(ws, 'setBrightness', devicePath, { brightness: 100 });
    }

    // N1 special functions
    if (deviceType === 'N1') {
      console.log('\n--- N1 special functions ---');

      // Test keyboard mode
      sendCommand(ws, 'changeN1Mode', devicePath, { mode: "keyboard" });
      for (let page = 1; page <= 5; page++) {
        console.log(`Changing to page ${page}`);
        sendCommand(ws, 'changeN1Page', devicePath, { page: page });
        await sleep(1000);
      }

      // Test calculator mode
      sendCommand(ws, 'changeN1Mode', devicePath, { mode: "calculator" });
      for (let page = 1; page <= 5; page++) {
        console.log(`Changing to page ${page}`);
        sendCommand(ws, 'changeN1Page', devicePath, { page: page });
        await sleep(1000);
      }

      // Switch back to dock mode
      sendCommand(ws, 'changeN1Mode', devicePath, { mode: "dock" });
      sendCommand(ws, 'refresh', devicePath, {});
    }

    // M3 special functions
    if (deviceType === 'M3') {
      console.log('\n--- M3 special functions ---');
      sendCommand(ws, 'setBackgroundGif', devicePath, {
        imagePath: TEST_BACKGROUND_GIF_PATH
      });
      await sleep(2000);

      // sendCommand(ws, 'magneticCalibration', devicePath, {});
    }

    // Mini special functions
    if (deviceType === 'Mini') {
      console.log('\n--- Mini special functions ---');
      sendCommand(ws, 'setLEDBrightness', devicePath, { brightness: 255 });
      sendCommand(ws, 'setLEDColor', devicePath, {
        r: 255,
        g: 0,
        b: 0
      });
      await sleep(1000);
    }

    // Set key GIFs/images for all image keys
    // i % 3 == 0 -> GIF, i % 3 == 1 -> button_test.jpg, i % 3 == 2 -> mark.png.
    console.log('\n--- Setting key GIFs/images ---');
    for (const i of imageKeysForDevice(deviceType)) {
      if (keyGifSupported && i % 3 === 0) {
        sendCommand(ws, 'setKeyGif', devicePath, {
          keyIndex: i,
          imagePath: TEST_KEY_GIF_PATH
        });
      } else if (i % 3 === 1) {
        sendCommand(ws, 'setKeyImg', devicePath, {
          keyIndex: i,
          imagePath: TEST_IMAGE_PATH
        });
      } else {
        sendCommand(ws, 'setKeyImg', devicePath, {
          keyIndex: i,
          imagePath: TEST_PNG_KEY_IMAGE_PATH
        });
        sendCommand(ws, 'refresh', devicePath, {});
      }
      await sleep(100);
    }
    sendCommand(ws, 'refresh', devicePath, {});
    await sleep(500);

    if (deviceType === 'H1Pro') {
      console.log('[H1Pro] Stage 8: start key GIF playback loop (independent of device GIF mode)');
    }
    if (keyGifSupported) {
      sendCommand(ws, 'startGifLoop', devicePath, {});
    }

    // Start input event listener
    console.log(deviceType === 'H1Pro' ? '\n[H1Pro] Stage 9: listen for key events' : '\n--- Starting input event listener ---');
    console.log('Press keys, rotate knobs, swipe, touch N4Pro touch bar, or toggle XL/Mini DIP switches to see events...');
    console.log('Press Ctrl+C to stop\n');

    sendCommand(ws, 'read', devicePath, {});

    ws.on('message', (data) => {
      const msg = JSON.parse(data);

      if (msg.event === 'read') {
        const payload = msg.payload;

        // Button events
        if (payload.keyId !== undefined) {
          const action = payload.keyUpOrKeyDown === 'keyDown' ? 'pressed' : 'released';
          console.log(`Key ${payload.keyId} ${action}`);
        }

        // Knob rotation events
        if (payload.knobId !== undefined && payload.direction !== undefined) {
          console.log(`Knob ${payload.knobId} rotated ${payload.direction}`);
        }

        // Knob press events
        if (payload.knobId !== undefined && payload.state !== undefined) {
          console.log(`Knob ${payload.knobId} ${payload.state}`);
        }

        // XL/Mini DIP switch events
        if (payload.type === 'dip_switch') {
          const direction = payload.direction ? ` ${payload.direction}` : '';
          console.log(`DIP switch ${payload.dipId}${direction} ${payload.state} (${payload.rawState})`);
        }

        // Swipe events
        if (
          payload.direction !== undefined &&
          payload.keyId === undefined &&
          payload.knobId === undefined &&
          payload.type === undefined
        ) {
          console.log(`Swipe gesture: ${payload.direction}`);
        }

        // N4Pro touch point events
        if (payload.type === 'touch_point') {
          console.log(`N4Pro touch point: (${payload.x}, ${payload.y})`);
        }
      }
    });

    // Keep running until interrupted
    await new Promise((resolve) => {
      process.on('SIGINT', () => {
        console.log('\nShutting down...');

        // Clear callback and close
        console.log(`Closing device ${deviceInfo.Path}...`);
        ws.close();
        resolve();
      });
    });

    console.log('\nComprehensive test completed!');
    return true;
  } catch (error) {
    console.error('Comprehensive test failed:', error.message);
    return false;
  }
}

// =============================================================================
// MAIN - RUN TESTS
// =============================================================================

async function main() {
  console.log('=================================================');
  console.log('StreamDock WebSocket SDK Test Suite');
  console.log('=================================================');

  await testComprehensive();

  console.log('\n=================================================');
  console.log('All tests completed!');
  console.log('=================================================\n');
}

// Run the tests
main().catch(console.error);
