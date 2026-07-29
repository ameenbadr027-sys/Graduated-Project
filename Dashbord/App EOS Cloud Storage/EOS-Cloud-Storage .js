const PIN='1234';
let pin='',shots=0,drive=0,busy=false;

function kp(v){
  document.getElementById('pin-err').textContent='';
  if(v==='del') pin=pin.slice(0,-1);
  else if(v==='ok') check();
  else if(pin.length<4){pin+=v}
  dots();
  if(pin.length===4&&v!=='ok') setTimeout(check,300);
}

function dots(){
  for(let i=0;i<4;i++){
    const d=document.getElementById('d'+i);
    d.className='dot'+(i<pin.length?' on':'');
  }
}

function check(){
  if(pin===PIN){
    const ps=document.getElementById('pin-screen');
    ps.style.opacity='0';ps.style.transform='scale(.96)';
    ps.style.transition='all .5s ease';
    setTimeout(()=>{
      ps.style.display='none';
      const a=document.getElementById('app');
      a.style.display='block';
      a.style.animation='fadeup .5s ease';
      const t=new Date();
      document.getElementById('login-t').textContent=
        t.getHours()+':'+String(t.getMinutes()).padStart(2,'0');
    },500);
  } else {
    document.querySelectorAll('.dot').forEach(d=>{
      d.className='dot err';
      d.style.animation='shake .4s ease';
      setTimeout(()=>{d.style.animation='';},400);
    });
    document.getElementById('pin-err').textContent='Incorrect PIN — try again';
    pin='';setTimeout(dots,100);
  }
}

function now(){
  const t=new Date();
  return t.getHours()+':'+String(t.getMinutes()).padStart(2,'0')+':'+String(t.getSeconds()).padStart(2,'0');
}

function toggleFlash(){
  const on=document.getElementById('sw-flash').checked;
  document.getElementById('s-flash').textContent=on?'ON':'OFF';
  addLog(on?'var(--orange)':'var(--muted2)',on?'Flash: ON ⚡':'Flash: OFF');
  toast(on?'Flash is on ⚡':'Flash is off');
}

function capture(){
  if(busy) return;
  busy=true;
  document.getElementById('btn-cap').disabled=true;
  document.getElementById('cap-ring').classList.add('on');
  document.getElementById('rec').classList.add('on');
  document.getElementById('cam-txt').textContent='Capturing...';
  document.getElementById('ts-txt').textContent='Processing photo...';
  addLog('var(--cyan)','New photo capture requested');

  setTimeout(()=>{
    busy=false;shots++;
    if(document.getElementById('sw-auto').checked) drive++;
    document.getElementById('s-shots').textContent=shots;
    document.getElementById('s-drive').textContent=drive;
    document.getElementById('s-time').textContent=now().slice(0,5);
    document.getElementById('cap-ring').classList.remove('on');
    document.getElementById('rec').classList.remove('on');
    document.getElementById('cam-txt').textContent='✓ Capture complete';
    document.getElementById('ts-txt').textContent='Last photo: '+now();
    document.getElementById('btn-cap').disabled=false;
    addLog('var(--cyan)','Photo saved to Drive ☁️');
    toast('Captured and uploaded successfully ✓');
  },2800);
}

function openImg(){
  if(!shots){toast('Take a photo first');return}
  toast('Opening photo...');
}

function addLog(color,text){
  const list=document.getElementById('log-list');
  const el=document.createElement('div');
  el.className='log-item';
  el.innerHTML=`<div class="log-dot" style="background:${color}"></div><div><div class="log-t">${text}</div><div class="log-s">${now()}</div></div>`;
  list.insertBefore(el,list.firstChild);
  while(list.children.length>7) list.removeChild(list.lastChild);
}

function toast(msg){
  const t=document.getElementById('toast');
  document.getElementById('toast-txt').textContent=msg;
  t.classList.add('up');
  clearTimeout(t._t);
  t._t=setTimeout(()=>t.classList.remove('up'),2800);
}

function logout(){
  document.getElementById('app').style.display='none';
  const ps=document.getElementById('pin-screen');
  ps.style.display='flex';ps.style.opacity='0';ps.style.transform='scale(.96)';
  setTimeout(()=>{ps.style.transition='all .4s ease';ps.style.opacity='1';ps.style.transform='scale(1)'},10);
  pin='';dots();
  document.getElementById('pin-err').textContent='';
}