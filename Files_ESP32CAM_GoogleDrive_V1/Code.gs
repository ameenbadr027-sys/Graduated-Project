// ======================================================
// USER SETTINGS
// ======================================================

const SPREADSHEET_ID = "PASTE_YOUR_GOOGLE_SHEET_ID";
const DRIVE_FOLDER_ID = "PASTE_YOUR_DRIVE_FOLDER_ID";

const ESP32_KEY = "esp32_cam_key_123";
const DASHBOARD_PIN = "SET PIN";

const STATE_SHEET = "state";
const LOG_SHEET = "logs";

// ======================================================
// HELPERS
// ======================================================

function getSpreadsheet_() {
  return SpreadsheetApp.openById(SPREADSHEET_ID);
}

function defaultStateRaw_() {
  return {
    flashEnabled: false,
    captureNow: false,
    latestImageUrl: "",
    lastCaptureAt: "",
    lastReason: "",
    updatedAt: ""
  };
}

function ensureSheets_() {
  const ss = getSpreadsheet_();

  let state = ss.getSheetByName(STATE_SHEET);
  if (!state) state = ss.insertSheet(STATE_SHEET);

  if (state.getLastRow() === 0) {
    state.getRange("A1:F1").setValues([[
      "flashEnabled",
      "captureNow",
      "latestImageUrl",
      "lastCaptureAt",
      "lastReason",
      "updatedAt"
    ]]);

    state.getRange("A2:F2").setValues([[
      false,
      false,
      "",
      "",
      "",
      new Date()
    ]]);
  }

  let logs = ss.getSheetByName(LOG_SHEET);
  if (!logs) logs = ss.insertSheet(LOG_SHEET);

  if (logs.getLastRow() === 0) {
    logs.getRange("A1:F1").setValues([[
      "timestamp",
      "source",
      "flashEnabled",
      "captureNow",
      "latestImageUrl",
      "reason"
    ]]);
  }

  return { state, logs };
}

function boolVal_(v) {
  return v === true || String(v).toLowerCase() === "true" || String(v) === "1";
}

function formatDate_(d) {
  if (!d) return "";
  try {
    return Utilities.formatDate(
      new Date(d),
      Session.getScriptTimeZone(),
      "yyyy-MM-dd HH:mm:ss"
    );
  } catch (e) {
    return "";
  }
}

function toDateOrBlank_(v) {
  if (!v) return "";
  if (v instanceof Date) return v;
  const d = new Date(v);
  return isNaN(d.getTime()) ? "" : d;
}

function jsonOutput_(obj) {
  return ContentService
    .createTextOutput(JSON.stringify(obj))
    .setMimeType(ContentService.MimeType.JSON);
}

function isEsp32KeyValid_(params) {
  return String(params.key || "").trim() === ESP32_KEY;
}

function isPinValid_(pin) {
  return String(pin || "").trim() === DASHBOARD_PIN;
}

function currentStateRaw_() {
  const { state } = ensureSheets_();

  if (state.getLastRow() < 2) {
    return defaultStateRaw_();
  }

  const r = state.getRange("A2:F2").getValues()[0];

  return {
    flashEnabled: boolVal_(r[0]),
    captureNow: boolVal_(r[1]),
    latestImageUrl: String(r[2] || ""),
    lastCaptureAt: r[3] || "",
    lastReason: String(r[4] || ""),
    updatedAt: r[5] || ""
  };
}

function readState_() {
  const r = currentStateRaw_();

  return {
    flashEnabled: r.flashEnabled,
    captureNow: r.captureNow,
    latestImageUrl: r.latestImageUrl,
    lastCaptureAt: formatDate_(r.lastCaptureAt),
    lastReason: r.lastReason,
    updatedAt: formatDate_(r.updatedAt)
  };
}

function writeState_(patch, source, logIt) {
  const current = currentStateRaw_();
  const merged = Object.assign({}, current, patch || {});
  const now = new Date();

  const { state, logs } = ensureSheets_();

  state.getRange("A2:F2").setValues([[
    !!merged.flashEnabled,
    !!merged.captureNow,
    String(merged.latestImageUrl || ""),
    toDateOrBlank_(merged.lastCaptureAt),
    String(merged.lastReason || ""),
    now
  ]]);

  if (logIt !== false) {
    logs.appendRow([
      now,
      source || "unknown",
      !!merged.flashEnabled,
      !!merged.captureNow,
      String(merged.latestImageUrl || ""),
      String(merged.lastReason || "")
    ]);
  }

  return readState_();
}

// ======================================================
// WEB APP
// ======================================================

function doGet(e) {
  const params = (e && e.parameter) ? e.parameter : {};
  const action = String(params.action || "").toLowerCase();

  // ESP32 reads current control state
  if (action === "get") {
    if (!isEsp32KeyValid_(params)) {
      return jsonOutput_({ ok: false, error: "unauthorized" });
    }

    return jsonOutput_({
      ok: true,
      data: readState_()
    });
  }

  // Dashboard
  return HtmlService
    .createHtmlOutputFromFile("Index")
    .setTitle("ESP32-CAM Dashboard");
}

function doPost(e) {
  const params = (e && e.parameter) ? e.parameter : {};
  const action = String(params.action || "").toLowerCase();

  if (action !== "upload") {
    return jsonOutput_({ ok: false, error: "invalid_action" });
  }

  if (!isEsp32KeyValid_(params)) {
    return jsonOutput_({ ok: false, error: "unauthorized" });
  }

  let b64 = String((e.postData && e.postData.contents) || "").trim();
  b64 = b64.replace(/\s/g, "");

  if (!b64) {
    return jsonOutput_({ ok: false, error: "no_image_data" });
  }

  if (b64.startsWith("data:image")) {
    const commaIndex = b64.indexOf(",");
    if (commaIndex !== -1) {
      b64 = b64.substring(commaIndex + 1);
    }
  }

  const bytes = Utilities.base64Decode(b64);
  const ts = Utilities.formatDate(
    new Date(),
    Session.getScriptTimeZone(),
    "yyyyMMdd_HHmmss"
  );

  const reason = String(params.reason || "capture").replace(/[^a-zA-Z0-9_-]/g, "_");

  const folder = DriveApp.getFolderById(DRIVE_FOLDER_ID);
  const blob = Utilities.newBlob(bytes, MimeType.JPEG, `esp32cam_${reason}_${ts}.jpg`);
  const file = folder.createFile(blob);

  file.setSharing(DriveApp.Access.ANYONE_WITH_LINK, DriveApp.Permission.VIEW);

  const imageUrl = `https://drive.google.com/uc?export=view&id=${file.getId()}`;

  const data = writeState_(
    {
      latestImageUrl: imageUrl,
      captureNow: false,
      lastCaptureAt: new Date(),
      lastReason: reason
    },
    "upload",
    true
  );

  return jsonOutput_({
    ok: true,
    imageUrl: imageUrl,
    data: data
  });
}

// ======================================================
// DASHBOARD FUNCTIONS
// ======================================================

function verifyPin(pin) {
  return isPinValid_(pin);
}

function getStateForHtml(pin) {
  if (!isPinValid_(pin)) {
    throw new Error("Invalid PIN");
  }

  return readState_();
}

function setStateFromHtml(flashEnabled, captureNow, pin) {
  if (!isPinValid_(pin)) {
    throw new Error("Invalid PIN");
  }

  return writeState_(
    {
      flashEnabled: !!flashEnabled,
      captureNow: !!captureNow
    },
    "dashboard",
    true
  );
}