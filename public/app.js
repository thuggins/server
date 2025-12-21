

(function(){
  let main = document.getElementById('main');
  if (!main) {
    main = document.createElement('div');
    main.id = 'main';
    document.body.appendChild(main);
  }

  const nav = document.createElement('div');
  nav.id = 'nav';
  nav.style.marginBottom = '1rem';
  const pages = [
    { name: 'Home', id: 'home' },
    { name: 'About', id: 'about' },
    { name: 'Contact', id: 'contact' }
  ];
  let currentPage = 'home';
  let ws = null;
  let reconnectTimer = null;
  let reconnectDelay = 2000;
  let isReconnecting = false;

  function connectWS() {
    if (ws) {
      try { ws.close(); } catch (e) {}
      ws = null;
    }
    console.log('Attempting to connect WebSocket...');
    const wsProto = window.location.protocol === 'https:' ? 'wss://' : 'ws://';
    const wsPort = window.location.protocol === 'https:' ? '8443' : '8080';
    ws = new WebSocket(wsProto + 'localhost:' + wsPort + '/ws');
    ws.addEventListener('open', () => {
      console.log('WebSocket connected');
      main.innerHTML = '<p style="color:green">Connected to server</p>';
      if (currentPage) {
        ws.send(JSON.stringify({ type: 'page', page: currentPage }));
      }
      if (reconnectTimer) {
        clearTimeout(reconnectTimer);
        reconnectTimer = null;
      }
      isReconnecting = false;
    });
    ws.addEventListener('close', () => {
      console.log('WebSocket closed, will attempt to reconnect...');
      main.innerHTML = '<p style="color:red">Disconnected from server. Reconnecting...</p>';
      if (!isReconnecting) {
        isReconnecting = true;
        function tryReconnect() {
          if (ws && ws.readyState === WebSocket.OPEN) {
            isReconnecting = false;
            return;
          }
          connectWS();
          reconnectTimer = setTimeout(tryReconnect, reconnectDelay);
        }
        reconnectTimer = setTimeout(tryReconnect, reconnectDelay);
      }
    });
    ws.addEventListener('error', (e) => {
      console.log('WebSocket error:', e);
    });
    ws.addEventListener('message', (ev) => {
      main.innerHTML = ev.data;
    });
  }

  pages.forEach(page => {
    const btn = document.createElement('button');
    btn.textContent = page.name;
    btn.style.marginRight = '0.5rem';
    btn.onclick = () => {
      currentPage = page.id;
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ type: 'page', page: page.id }));
      }
    };
    nav.appendChild(btn);
  });
  document.body.insertBefore(nav, main);

  connectWS();
})();
