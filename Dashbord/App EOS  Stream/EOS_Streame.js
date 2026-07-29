const baseHost = document.location.origin;
    let streamUrl = baseHost + '/stream';
    let isStreaming = false;
    let flashOn = false;

    function logEvent(method, endpoint, params, status, response) {
      const consoleBox = document.getElementById('console-logs');
      if (!consoleBox) return;
      const now = new Date();
      const timeStr = now.toTimeString().split(' ')[0];
      const line = document.createElement('div');
      line.className = 'log-line';
      line.innerHTML = `<span class="log-timestamp">[${timeStr}]</span>
                       <span class="log-method" style="color: ${status === 200 ? 'var(--gold)' : 'var(--jewel-ruby-bright)'}">${method}</span>
                       <span class="log-text">${endpoint}${params ? '?' + params : ''} [${status}]</span>`;
      consoleBox.appendChild(line);
      consoleBox.scrollTop = consoleBox.scrollHeight;
      if (window.updateReactStats) window.updateReactStats({ lastResponse: `${method} ${endpoint} -> ${status}`, isStreaming });
    }

    function getStatus() {
      const endpoint = '/status';
      fetch(baseHost + endpoint)
        .then(response => { if (!response.ok) throw new Error('Status unavailable'); return response.json(); })
        .then(data => {
          logEvent('GET', endpoint, '', 200, 'Success');
          Object.keys(data).forEach(key => {
            const el = document.getElementById(key);
            if (!el) return;
            if (el.type === 'checkbox') {
              el.checked = data[key] === 1;
              if (key === 'face_detect') document.getElementById('enroll-btn').style.display = el.checked ? 'block' : 'none';
            } else {
              el.value = data[key];
              const valueLabel = document.getElementById(key + '-val');
              if (valueLabel) valueLabel.innerText = data[key];
            }
          });
        })
        .catch(err => { logEvent('GET', endpoint, '', 500, 'Offline template display configured'); });
    }

    function updateControl(name, value) {
      const endpoint = '/control';
      const params = `var=${name}&val=${value}`;
      if (name === 'face_detect') {
        const enrollBtn = document.getElementById('enroll-btn');
        if (enrollBtn) enrollBtn.style.display = parseInt(value) === 1 ? 'block' : 'none';
      }
      fetch(`${baseHost}${endpoint}?${params}`)
        .then(response => { logEvent('GET', endpoint, params, response.ok ? 200 : response.status, response.ok ? 'OK' : 'Fail'); })
        .catch(err => { logEvent('GET', endpoint, params, 200, '(SIMULATED_LOCAL_AP_MODE)'); });
    }

    function toggleStream() {
      const streamImg = document.getElementById('stream');
      const streamCover = document.getElementById('streamCover');
      const streamBtn = document.getElementById('stream-btn');

      if (!isStreaming) {
        isStreaming = true;
        streamImg.src = streamUrl;
        streamImg.style.display = 'block';
        streamCover.style.display = 'none';
        streamBtn.innerText = 'Stop Stream';
        streamBtn.className = 'btn btn-danger';
        logEvent('GET', '/stream', '', 200, 'Stream started');
      } else {
        isStreaming = false;
        streamImg.src = '';
        streamImg.style.display = 'none';
        streamCover.style.display = 'flex';
        streamBtn.innerText = 'Start Stream';
        streamBtn.className = 'btn btn-success';
        logEvent('GET', '/stream', 'stop', 200, 'Stream halted');
      }
      const apertureMark = document.getElementById('apertureMark');
      if (apertureMark) apertureMark.classList.toggle('is-live', isStreaming);

      if (window.updateReactStats) window.updateReactStats({ isStreaming });
    }

    function captureSnapshot() {
      const endpoint = '/capture';
      logEvent('GET', endpoint, '', 200, 'Snapshot opened');
      window.open(baseHost + endpoint, '_blank');
    }

    function enrollFace() { updateControl('face_enroll', 1); }

    function toggleFlash() {
      flashOn = !flashOn;
      const endpoint = '/flash';
      const val = flashOn ? 1 : 0;
      const params = `v=${val}`;
      const btn = document.getElementById('flash-toggle-btn');

      fetch(`${baseHost}${endpoint}?${params}`)
        .then(response => { logEvent('GET', endpoint, params, response.status, 'Response received'); })
        .catch(err => { logEvent('GET', endpoint, params, 200, '(SIMULATED_FLASH)'); });

      if (flashOn) {
        btn.innerText = 'Flash — On';
        btn.style.boxShadow = '0 0 14px var(--gold)';
        btn.style.color = 'var(--gold-bright)';
      } else {
        btn.innerText = 'Flash — Off';
        btn.style.boxShadow = '';
        btn.style.color = '';
      }
      if (window.updateReactStats) window.updateReactStats({ flashActive: flashOn });
    }

    function restartDevice() {
      if (confirm('Initiate ESP32 warm reboot sequence?')) {
        const endpoint = '/restart';
        logEvent('GET', endpoint, '', 200, 'Reboot dispatch');
        fetch(baseHost + endpoint)
          .then(() => alert('Rebooting… Please reload this page in 5 seconds.'))
          .catch(() => alert('System command dispatched. Reconnecting board in 5 seconds.'));
      }
    }

    function clearLogs() {
      const consoleBox = document.getElementById('console-logs');
      if (consoleBox) consoleBox.innerHTML = '';
    }

    // ---- Cloud Vault (Google Drive backup) ----
    function renderVaultRecord(data) {
      const valueEl = document.getElementById('vault-record-value');
      const pillEl = document.getElementById('vault-pill');
      if (!valueEl || !pillEl) return;

      if (data && data.imageUrl) {
        valueEl.classList.remove('empty');
        valueEl.innerHTML =
          (data.reason ? 'Reason: ' + data.reason + '<br>' : '') +
          (data.at ? 'Captured: ' + data.at + '<br>' : '') +
          `<a href="${data.imageUrl}" target="_blank" rel="noopener">Open in Drive</a>`;
        pillEl.textContent = 'Drive · Linked';
        pillEl.classList.add('linked');
      } else {
        valueEl.classList.add('empty');
        valueEl.textContent = 'No backup yet this session.';
        pillEl.textContent = 'Drive · Not Linked';
        pillEl.classList.remove('linked');
      }
    }

    function pollBackupStatus() {
      fetch(`${baseHost}/backup_status`)
        .then(response => { if (!response.ok) throw new Error('unavailable'); return response.json(); })
        .then(data => renderVaultRecord(data))
        .catch(() => {});
    }

    function backupToDrive() {
      const endpoint = '/backup';
      const btn = document.getElementById('vault-backup-btn');
      btn.disabled = true;
      btn.innerText = 'Uploading…';
      logEvent('GET', endpoint, 'reason=manual', 200, 'Backup requested');

      fetch(`${baseHost}${endpoint}?reason=manual`)
        .then(response => response.json())
        .then(data => {
          logEvent('GET', endpoint, 'reason=manual', 200, data.ok ? 'Uploaded' : 'Upload failed');
          if (data.ok) renderVaultRecord({ imageUrl: data.imageUrl, reason: 'manual', at: 'just now' });
        })
        .catch(() => logEvent('GET', endpoint, 'reason=manual', 200, '(OFFLINE_PREVIEW — connect to a live EOS board to upload)'))
        .finally(() => {
          btn.disabled = false;
          btn.innerText = 'Backup to Drive';
        });
    }

    if (document.location.hostname) {
      document.getElementById('local-ip-badge').innerText = document.location.hostname;
    }

    window.onload = function () { getStatus(); pollBackupStatus(); };

(function () {
      const container = document.getElementById('react-hud-island');

      function renderVanillaFallback() {
        container.innerHTML = `
          <div style="padding: 14px; display: grid; grid-template-columns: 1fr 1fr; gap: 8px;">
            <div style="border-right: 1px solid var(--border); padding-right:10px;">
              <div style="font-size: 9px; color: var(--ink-faint); text-transform: uppercase; letter-spacing:0.12em;">Device Uptime</div>
              <div id="v-uptime" style="font-size: 14px; font-weight: 600; color: var(--ink); font-family: var(--font-mono); margin-top:3px;">00:00:00</div>
            </div>
            <div>
              <div style="font-size: 9px; color: var(--ink-faint); text-transform: uppercase; letter-spacing:0.12em;">Console Clock</div>
              <div id="v-clock" style="font-size: 14px; font-weight: 600; color: var(--gold); font-family: var(--font-mono); margin-top:3px;">00:00:00</div>
            </div>
          </div>
          <div style="font-size: 9px; padding: 8px 14px; border-top: 1px solid var(--border); color: var(--ink-faint); text-align: right; letter-spacing:0.1em; text-transform: uppercase;">
            Vanilla Offline Engine Active
          </div>`;
        const startTime = Date.now();
        setInterval(() => {
          const secElapsed = Math.floor((Date.now() - startTime) / 1000);
          const hrs = Math.floor(secElapsed / 3600).toString().padStart(2, '0');
          const mins = Math.floor((secElapsed % 3600) / 60).toString().padStart(2, '0');
          const secs = (secElapsed % 60).toString().padStart(2, '0');
          const uptimeEl = document.getElementById('v-uptime');
          if (uptimeEl) uptimeEl.innerText = `${hrs}:${mins}:${secs}`;
          const nowObj = new Date();
          const clockEl = document.getElementById('v-clock');
          if (clockEl) clockEl.innerText = nowObj.toTimeString().split(' ')[0];
        }, 1000);
      }

      if (typeof React === 'undefined' || typeof ReactDOM === 'undefined') { renderVanillaFallback(); return; }

      try {
        const e = React.createElement;

        function ReactHudIsland() {
          const [uptime, setUptime] = React.useState(0);
          const [timeString, setTimeString] = React.useState('00:00:00');
          const [connStats, setConnStats] = React.useState({ lastResponse: 'System Ready', isStreaming: false, flashActive: false });

          React.useEffect(() => {
            const timer = setInterval(() => {
              setUptime(prev => prev + 1);
              setTimeString(new Date().toTimeString().split(' ')[0]);
            }, 1000);
            window.updateReactStats = (statsUpdate) => setConnStats(prev => ({ ...prev, ...statsUpdate }));
            return () => clearInterval(timer);
          }, []);

          const formatUptime = (totalSeconds) => {
            const h = Math.floor(totalSeconds / 3600).toString().padStart(2, '0');
            const m = Math.floor((totalSeconds % 3600) / 60).toString().padStart(2, '0');
            const s = (totalSeconds % 60).toString().padStart(2, '0');
            return `${h}:${m}:${s}`;
          };

          return e('div', null,
            e('div', { style: { padding: '14px', display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '8px' } },
              e('div', { style: { borderRight: '1px solid var(--border)', paddingRight: '10px' } },
                e('div', { style: { fontSize: '9px', color: 'var(--ink-faint)', textTransform: 'uppercase', letterSpacing: '0.12em' } }, 'Device Uptime'),
                e('div', { style: { fontSize: '14px', fontWeight: 600, color: 'var(--ink)', fontFamily: 'var(--font-mono)', marginTop: '3px' } }, formatUptime(uptime))
              ),
              e('div', null,
                e('div', { style: { fontSize: '9px', color: 'var(--ink-faint)', textTransform: 'uppercase', letterSpacing: '0.12em' } }, 'Console Clock'),
                e('div', { style: { fontSize: '14px', fontWeight: 600, color: 'var(--gold)', fontFamily: 'var(--font-mono)', marginTop: '3px' } }, timeString)
              )
            ),
            e('div', { style: { fontSize: '9px', padding: '8px 14px', borderTop: '1px solid var(--border)', color: 'var(--ink-faint)', display: 'flex', justifyContent: 'space-between', letterSpacing: '0.1em', textTransform: 'uppercase' } },
              e('span', null, `Link: ${connStats.isStreaming ? 'Active MJPEG' : 'Standby'}`),
              e('span', { style: { color: 'var(--gold)', fontWeight: 600 } }, 'React Engine Online')
            )
          );
        }

        const root = ReactDOM.createRoot(container);
        root.render(e(ReactHudIsland));
      } catch (err) {
        console.error('React Initialization failed, reverting to vanilla:', err);
        renderVanillaFallback();
      }
    })();