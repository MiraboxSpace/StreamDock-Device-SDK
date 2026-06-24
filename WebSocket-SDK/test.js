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
//
// Usage:
// - Uncomment the test section you want to run
// - Run with: node docs/test.js
// =============================================================================

const WebSocket = require('ws');
const path = require('path');
const fs = require('fs');

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
  console.log(`Sending: ${event}`);
  ws.send(JSON.stringify(command));
}

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

function keyCountForDevice(deviceType) {
  const keyCounts = {
    '293': 15,
    '293V3': 15,
    '293s': 6,
    '293sV3': 6,
    'N3': 9,
    'N3EN': 9,
    'N4': 14,
    'N4EN': 14,
    'N1': 15,
    'N1EN': 15,
    'N4Pro': 14,
    'XL': 32,
    'M18': 18,
    'M3': 15,
    'K1Pro': 6,
    'Mini': 6
  };
  return keyCounts[deviceType] || 18;
}

function imageKeysForDevice(deviceType) {
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
      console.log('Received:', msg);

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
    const { ws, devicePath, deviceInfo } = await createConnectionAndListen();

    console.log(`\nDevice Info:`);
    console.log(`  Path: ${deviceInfo.Path}`);
    console.log(`  Type: ${deviceInfo.Type}`);
    console.log(`  Firmware: ${deviceInfo.FirmwareVersion}`);
    console.log(`  Serial: ${deviceInfo.SerialNumber}`);

    // Set background image
    // The image will be written to ROM. Keep this disabled to match Python SDK main.py.
    // sendCommand(ws, 'setBackgroundImg', devicePath, {
    //   imagePath: TEST_BACKGROUND_PATH
    // });
    sendCommand(ws, 'refresh', devicePath, {});
    await sleep(2000);

    // Device-specific tests
    const deviceType = deviceInfo.Type;

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
    // Matches Python SDK main.py:
    // i % 3 == 0 -> GIF, i % 3 == 1 -> button_test.jpg, i % 3 == 2 -> mark.png.
    console.log('\n--- Setting key GIFs/images ---');
    for (const i of imageKeysForDevice(deviceType)) {
      if (i % 3 === 0) {
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

    sendCommand(ws, 'startGifLoop', devicePath, {});

    // Start input event listener
    console.log('\n--- Starting input event listener ---');
    console.log('Press keys, rotate knobs, swipe, touch N4Pro touch bar, or toggle Mini DIP switches to see events...');
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

        // Mini DIP switch events
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
