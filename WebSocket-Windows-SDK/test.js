// =============================================================================
// StreamDock WebSocket SDK - Complete Test Suite
// =============================================================================
// This file demonstrates all available WebSocket commands for StreamDock devices
//
// Requirements:
// - Node.js environment
// - WebSocket server running on ws://127.0.0.1:9002
// - ws package: npm install ws
// - Test images in E:/img/ directory (or update paths below)
//
// Usage:
// - Uncomment the test section you want to run
// - Run with: node "Test Websocket Command.js"
// =============================================================================

const WebSocket = require('ws');

// =============================================================================
// CONFIGURATION
// =============================================================================

const WS_URL = 'ws://127.0.0.1:9002';
const TEST_IMAGE_PATH = "button_test.jpg";
const TEST_BACKGROUND_PATH = 'background_test.png';

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Base64 encode a string
 */
function base64Encode(str) {
  return Buffer.from(str).toString('base64');
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
  console.log('\n=== TEST 9: COMPREHENSIVE TEST (Like Python SDK) ===');

  try {
    const { ws, devicePath, deviceInfo } = await createConnectionAndListen();

    console.log(`\nDevice Info:`);
    console.log(`  Path: ${deviceInfo.Path}`);
    console.log(`  Type: ${deviceInfo.Type}`);
    console.log(`  Firmware: ${deviceInfo.FirmwareVersion}`);
    console.log(`  Serial: ${deviceInfo.SerialNumber}`);

    // Set background image
    console.log('\n--- Setting background image ---');
    sendCommand(ws, 'setBackgroundImg', devicePath, {
      imagePath: TEST_BACKGROUND_PATH
    });
    sendCommand(ws, 'refresh', devicePath, {});
    await new Promise(resolve => setTimeout(resolve, 1000));

    // Device-specific tests
    const deviceType = deviceInfo.Type;

    // N4Pro special functions
    if (deviceType === 'N4Pro') {
      console.log('\n--- N4Pro special functions ---');
      sendCommand(ws, 'setLEDBrightness', devicePath, { brightness: 100 });
      sendCommand(ws, 'setLEDColor', devicePath, { r: 0, g: 0, b: 255 });
      await new Promise(resolve => setTimeout(resolve, 2000));
    }

    // XL special functions
    if (deviceType === 'XL') {
      console.log('\n--- XL special functions ---');
      await new Promise(resolve => setTimeout(resolve, 2000));
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
        await new Promise(resolve => setTimeout(resolve, 1000));
      }

      // Test calculator mode
      sendCommand(ws, 'changeN1Mode', devicePath, { mode: "calculator" });
      for (let page = 1; page <= 5; page++) {
        console.log(`Changing to page ${page}`);
        sendCommand(ws, 'changeN1Page', devicePath, { page: page });
        await new Promise(resolve => setTimeout(resolve, 1000));
      }

      // Switch back to dock mode
      sendCommand(ws, 'changeN1Mode', devicePath, { mode: "dock" });
      sendCommand(ws, 'refresh', devicePath, {});
    }

    // M3 special functions
    if (deviceType === 'M3') {
      console.log('\n--- M3 special functions ---');
      await new Promise(resolve => setTimeout(resolve, 2000));

      console.log('Performing magnetic calibration...');
      sendCommand(ws, 'magneticCalibration', devicePath, {});
    }

    // Set key images for all keys
    console.log('\n--- Setting key images ---');
    let maxKeys = 32;
    switch (deviceType) {
      case 'N1':
        maxKeys = 18;
        break;
      case 'N4Pro':
        maxKeys = 14;
        break;
      case 'XL':
        maxKeys = 32;
        break;
      case 'K1Pro':
        maxKeys = 6;
        break;
      default:
        maxKeys = 18;
        break;
    }
    for (let i = 1; i <= maxKeys; i++) {
      sendCommand(ws, 'setKeyImg', devicePath, {
        keyIndex: i,
        imagePath: TEST_IMAGE_PATH
      });
      await new Promise(resolve => setTimeout(resolve, 100));
    }
    sendCommand(ws, 'refresh', devicePath, {});
    await new Promise(resolve => setTimeout(resolve, 500));

    // Start input event listener
    console.log('\n--- Starting input event listener ---');
    console.log('Press keys, rotate knobs, or swipe to see events...');
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

        // Swipe events
        if (payload.direction !== undefined && payload.keyId === undefined) {
          console.log(`Swipe gesture: ${payload.direction}`);
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
