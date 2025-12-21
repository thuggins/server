(function(){
  const status = document.getElementById('status');
  const logEl = document.getElementById('log');
  const input = document.getElementById('msg');
  const btnSend = document.getElementById('send');
  const btnPing = document.getElementById('ping');

  function log(msg){
    const time = new Date().toLocaleTimeString();
    logEl.textContent += `[${time}] ${msg}\n`;
    logEl.scrollTop = logEl.scrollHeight;
  }

  const ws = new WebSocket('ws://localhost:8080/ws');
  ws.addEventListener('open', () => {
    status.textContent = 'connected';
    log('WebSocket connected');
  });
  ws.addEventListener('close', () => {
    status.textContent = 'disconnected';
    log('WebSocket closed');
  });
  ws.addEventListener('message', (ev) => {
    let data = ev.data;
    try { data = JSON.parse(data); } catch {}
    if (typeof data === 'object' && data) {
      if (data.type === 'welcome') log(`Server: ${data.msg}`);
      else if (data.type === 'echo') log(`Echo: ${data.msg}`);
      else log(`Server: ${JSON.stringify(data)}`);
    } else {
      log(`Server: ${data}`);
    }
  });

  btnSend.addEventListener('click', () => {
    if (ws.readyState === WebSocket.OPEN) {
      const text = input.value || '(empty)';
      ws.send(text);
      log(`Sent: ${text}`);
      input.value = '';
    }
  });

  btnPing.addEventListener('click', () => {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send('ping');
      log('Sent: ping');
    }
  });
})();
