// Page web servie par l'ESP8266 sur "/". Elle interroge /data toutes les 2 secondes.
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Maison intelligente</title>
  <link href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" rel="stylesheet">
  <style>
    body {
      margin: 0;
      font-family: Arial, sans-serif;
      color: #333;
      background: linear-gradient(45deg, #FFC700, #FFB300, #FF9100, #FF6F00, #FF9100, #FFC700);
      background-size: 400% 400%;
      animation: fond 15s ease infinite;
      min-height: 100vh;
    }
    @keyframes fond { 0% { background-position: 0% 50%; } 50% { background-position: 100% 50%; } 100% { background-position: 0% 50%; } }
    .container { max-width: 620px; margin: 0 auto; padding: 20px 16px; }
    h1 { font-size: 2.4rem; color: #fff; text-align: center; text-shadow: 2px 2px 4px rgba(0,0,0,.45); margin: 10px 0 20px; }
    .card { background: rgba(255,255,255,.88); border-radius: 14px; padding: 14px 18px; margin-bottom: 14px;
            display: flex; align-items: center; gap: 16px; box-shadow: 0 4px 14px rgba(0,0,0,.12); }
    .card i { font-size: 2.2rem; width: 46px; text-align: center; }
    .label { flex: 1; font-size: 1.2rem; }
    .value { font-size: 1.5rem; font-weight: bold; }
    .unit { font-size: 1rem; margin-left: 2px; }
    .badge { padding: 4px 12px; border-radius: 999px; font-size: .95rem; font-weight: bold; color: #fff; background: #4CAF50; }
    .badge.alerte { background: #e53935; }
    .lampe .etat { font-size: .95rem; color: #666; }
    button { padding: 9px 16px; border: none; border-radius: 8px; color: #fff; font-size: 1rem; cursor: pointer; }
    .on { background: #4CAF50; }
    .off { background: #757575; }
    .alarme { animation: clignote 1s infinite; }
    @keyframes clignote { 50% { opacity: .35; } }
    .horloge { text-align: center; color: #fff; text-shadow: 1px 1px 2px rgba(0,0,0,.45); font-size: 1rem; }
  </style>
</head>
<body>
  <div class="container">
    <h1>Maison intelligente</h1>

    <div class="card">
      <i class="fas fa-thermometer-half" style="color:#62a1d3"></i>
      <span class="label">Température</span>
      <span><span class="value" id="temperature">--</span><span class="unit">&deg;C</span></span>
      <span class="badge" id="tempAlarm">Normal</span>
    </div>

    <div class="card">
      <i class="fas fa-tint" style="color:#4fc3a1"></i>
      <span class="label">Humidité</span>
      <span><span class="value" id="humidity">--</span><span class="unit">%</span></span>
    </div>

    <div class="card">
      <i class="fas fa-burn" style="color:#f5a623" id="gasIcon"></i>
      <span class="label">Gaz (MQ2)<br><small id="gasThreshold"></small></span>
      <span class="value" id="gas">--</span>
      <span class="badge" id="gasAlarm">Normal</span>
    </div>

    <div class="card lampe">
      <i class="fas fa-lightbulb" id="led1Icon" style="color:#bbb"></i>
      <span class="label">Lampe 1<br><span class="etat" id="led1">--</span></span>
      <button class="on" onclick="lampe('led1', 'on')">Allumer</button>
      <button class="off" onclick="lampe('led1', 'off')">Éteindre</button>
    </div>

    <div class="card lampe">
      <i class="fas fa-lightbulb" id="led2Icon" style="color:#bbb"></i>
      <span class="label">Lampe 2<br><span class="etat" id="led2">--</span></span>
      <button class="on" onclick="lampe('led2', 'on')">Allumer</button>
      <button class="off" onclick="lampe('led2', 'off')">Éteindre</button>
    </div>

    <p class="horloge"><i class="far fa-clock"></i> <span id="heure"></span> &nbsp; <i class="far fa-calendar-alt"></i> <span id="date"></span></p>
  </div>

  <script>
    function afficher(d) {
      document.getElementById("temperature").textContent = d.temperature === null ? "--" : d.temperature.toFixed(1);
      document.getElementById("humidity").textContent = d.humidity === null ? "--" : d.humidity.toFixed(0);
      document.getElementById("gas").textContent = d.gas;
      document.getElementById("gasThreshold").textContent = "seuil d'alarme : " + d.gasThreshold;
      badge("gasAlarm", d.gasAlarm, "Alarme !");
      badge("tempAlarm", d.tempAlarm, "Chaleur !");
      document.getElementById("gasIcon").className = "fas fa-burn" + (d.gasAlarm ? " alarme" : "");
      etatLampe("led1", d.led1);
      etatLampe("led2", d.led2);
    }
    function badge(id, alerte, texte) {
      var el = document.getElementById(id);
      el.textContent = alerte ? texte : "Normal";
      el.className = "badge" + (alerte ? " alerte" : "");
    }
    function etatLampe(id, allumee) {
      document.getElementById(id).textContent = allumee ? "Allumée" : "Éteinte";
      document.getElementById(id + "Icon").style.color = allumee ? "#FFC107" : "#bbb";
    }
    function rafraichir() {
      fetch("/data").then(function (r) { return r.json(); }).then(afficher).catch(function () {});
    }
    function lampe(id, etat) {
      fetch("/" + id + "?state=" + etat).then(rafraichir);
    }
    function horloge() {
      var d = new Date();
      document.getElementById("heure").textContent = d.toLocaleTimeString("fr-FR");
      document.getElementById("date").textContent = d.toLocaleDateString("fr-FR", { weekday: "long", day: "numeric", month: "long", year: "numeric" });
    }
    rafraichir();
    horloge();
    setInterval(rafraichir, 2000);
    setInterval(horloge, 1000);
  </script>
</body>
</html>
)=====";
