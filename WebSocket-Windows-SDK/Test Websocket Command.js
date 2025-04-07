{
  // connect
  const ws = new WebSocket('ws://127.0.0.1:9002')
  let path = null;
  ws.onopen = () => {
    console.log("open")
  };

  ws.onmessage = (e) => {
    const a = JSON.parse(e.data)
    path = a.path
    console.log(JSON.parse(e.data))
  };

  ws.onerror = () => {
    ws.close(); // close the connection to trigger `onclose`
  };

  ws.onclose = () => {
    console.log("close")
  };
}

{
  // 发送图片
  const setkey = {
    "event": "setKeyImg",
    "path": path,
    "payload": {
      "url": "E:\\img\\中文.jpg",    // unicode
      "key": 1,
      "refresh": true
    }
  };
  ws.send(JSON.stringify(setkey));

  const refresh =
  {
    "event": "refresh",
    "path": path
  }
  ws.send(JSON.stringify(refresh));
}

{
  // read
  const read = {
    "event": "read",
    "path": path
  };
  ws.send(JSON.stringify(read));
}

{
  // connect
  const ws = new WebSocket('ws://127.0.0.1:9002')
  let path = null;
  ws.onopen = () => {
    console.log("open")
  };
  ws.onmessage = (e) => {
    const a = JSON.parse(e.data)
    path = a.path
    console.log(JSON.parse(e.data))
  };
  ws.onerror = () => {
    ws.close(); // close the connection to trigger `onclose`
  };
  ws.onclose = () => {
    console.log("close")
  };
  
  // read
  const read = {
    "event": "read",
    "path": path
  };
  ws.send(JSON.stringify(read));
  // knob receive something and do something(set picture)
  ws.onmessage = (e) => {
    const a = JSON.parse(e.data)
    path = a.path
    // 发送图片
    const setkey = {
      "event": "setKeyImg",
      "path": path,
      "payload": {
        "url": "E:\\img\\中文.jpg",
        "key": 1,
        "refresh": true
      }
    };
    ws.send(JSON.stringify(setkey));
    const refresh =
    {
      "event": "refresh",
      "path": path
    }
    ws.send(JSON.stringify(refresh));
  };
}

{
  // set background image(openning logo)
  const setBackgroundImg = {
    "event": "setBackgroundImg",
    "path": path,
    "payload": {
      "url": "E:\\img\\bg.jpg",
      "key": 1
    }
  };
  ws.send(JSON.stringify(setBackgroundImg));
  const refresh =
  {
    "event": "refresh",
    "path": path
  }
  ws.send(JSON.stringify(refresh));
}

{
  // disconnected
  const disconnected = {
    "event": "disconnected",
    "path": path,
  };
  ws.send(JSON.stringify(disconnected));
}