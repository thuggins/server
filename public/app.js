


(function(){
  let main = document.getElementById('main');
  if (!main) {
    main = document.createElement('div');
    main.id = 'main';
    document.body.appendChild(main);
  }
  main.innerHTML = `
    <div style="display:flex;margin:2em 0;">
      <button id="weatherBtn" style="background:#1976d2;color:#fff;border:none;padding:12px 28px;font-size:1.1em;border-radius:6px;box-shadow:0 2px 8px #0002;cursor:pointer;transition:background 0.2s;">Get Weather</button>
    </div>
    <div id="weather" style="max-width:500px;"></div>
  `;

  const log = null;
  const weatherBtn = document.getElementById('weatherBtn');
  const weatherDiv = document.getElementById('weather');
  let ws = null;
  let reconnectTimer = null;
  let reconnectDelay = 2000;
  let isReconnecting = false;

  function setWeatherLoading(loading) {
    if (weatherBtn) weatherBtn.disabled = loading;
    if (weatherDiv) weatherDiv.textContent = loading ? 'Loading weather...' : '';
  }

  function connectWS() {
    if (ws) {
      try { ws.close(); } catch (e) {}
      ws = null;
    }
    const wsProto = window.location.protocol === 'https:' ? 'wss://' : 'ws://';
    const wsPort = window.location.protocol === 'https:' ? '8443' : '8080';
    ws = new WebSocket(wsProto + 'localhost:' + wsPort + '/ws');
    ws.addEventListener('open', () => {
      setWeatherLoading(false);
      if (reconnectTimer) {
        clearTimeout(reconnectTimer);
        reconnectTimer = null;
      }
      isReconnecting = false;
    });
    ws.addEventListener('close', () => {
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
      // No log output
    });
    // Store rows as they arrive
    let weatherRows = [];
    ws.addEventListener('message', (ev) => {
      try {
        const data = JSON.parse(ev.data);
        if (data.type === 'weather_result') {
          setWeatherLoading(false);
          if (weatherDiv) {
            weatherDiv.textContent = `Weather: ${data.weather} (${data.temp}°C)`;
          }
          return;
        }
        if (data.type === 'weather_table' && Array.isArray(data.data)) {
          setWeatherLoading(false);
          weatherRows = data.data;
          renderWeatherTable();
          return;
        }
        if (data.type === 'weather_row') {
          setWeatherLoading(false);
          weatherRows.push({city: data.city, state: data.state, temp: data.temp});
          renderWeatherTable();
          return;
        }
      } catch (e) {}
      // No log output for other messages
    });

    function renderWeatherTable() {
      if (!weatherDiv) return;
      if (weatherRows.length === 0) {
        weatherDiv.innerHTML = '';
        return;
      }
        let html = `<table style="border-collapse:collapse;min-width:340px;box-shadow:0 2px 8px #0001;border-radius:8px;overflow:hidden;">
          <thead><tr style="background:#1976d2;color:#fff;"><th style="padding:8px 16px;">City</th><th style="padding:8px 16px;">State</th><th style="padding:8px 16px;">Temp (°C)</th></tr></thead><tbody>`;
      for (const row of weatherRows) {
          html += `<tr style="background:#fff;"><td style="padding:8px 16px;border-bottom:1px solid #eee;">${row.city}</td><td style="padding:8px 16px;border-bottom:1px solid #eee;">${row.state}</td><td style="padding:8px 16px;border-bottom:1px solid #eee;text-align:right;">${row.temp}</td></tr>`;
      }
        html += '</tbody></table>';
      weatherDiv.innerHTML = html;
    }

    if (weatherBtn) {
      weatherBtn.onclick = () => {
        if (ws && ws.readyState === WebSocket.OPEN) {
          setWeatherLoading(true);
          weatherRows = [];
          renderWeatherTable();
            // Clear table immediately and show loading
            if (weatherDiv) {
              weatherDiv.innerHTML = '<div style="padding:1em;text-align:center;color:#888;">Loading weather...</div>';
            }
            ws.send(JSON.stringify({ type: 'weather_request' }));
        }
      };
    }
  }

  connectWS();
})();
