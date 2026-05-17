function setActiveNav(activeBtn) {
  document.querySelectorAll(".nav-item").forEach(btn => {
    btn.classList.remove("nav-item--active");
  });
  activeBtn.classList.add("nav-item--active");
}

function showView(viewId, navBtn) {
  document
    .querySelectorAll("#dashboard-view, #settings-view")
    .forEach(v => v.classList.add("hidden"));
  document.getElementById(viewId).classList.remove("hidden");
  setActiveNav(navBtn);
}



window.onload = async function () {

  const now = new Date();

  const h   = now.getHours();
  const min = now.getMinutes();
  const ss  = now.getSeconds();

  const d   = now.getDate();
  const mon = now.getMonth() + 1;
  const yy  = now.getFullYear() % 100;

  console.log(`Browser Time: ${h}:${min}:${ss}`);
  console.log(`Browser Date: ${d}/${mon}/${yy}`);

  // Send to ESP32
  try {

    await fetch('/settings_set_settings_data', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(`&${CODEX.UPDATE_TIME}&${h}&${min}&${ss}&`)
    });

    await fetch('/settings_set_settings_data', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify(`&${CODEX.UPDATE_DATE}&${d}&${mon}&${yy}&`)
    });

    console.log("RTC updated successfully");

  } catch (err) {

    console.error("Failed to update RTC:", err);

  }
};


async function renderDashboard() {

  const wifiSelect = document.getElementById("dashboard-device-wifi");
  const wifiPass = document.getElementById("dashboard-device-pass");
  const wifiConnectBtn = document.getElementById("wifi-btn");
  // do not touch
  var resp;
  resp = await getSettingsUserName();
  systemSettings.user_name = resp.toUpperCase();
  document.getElementById("dash-username").textContent = systemSettings.user_name;
  document.querySelector("#dash-avatar span").textContent = systemSettings.user_name[0];

  resp = await getSettingsDeviceName();
  systemSettings.device_name = resp.toUpperCase();
  document.getElementById("dash-subtitle").textContent = systemSettings.device_name;

  resp = await getSettingsDeviceLang();
  systemSettings.device_lang = resp;

  resp = await getSettingsDeviceUnit();
  systemSettings.units = resp;

  resp = await getSettingsDeviceUnit();
  systemSettings.theme = resp;
  // end do not touch

  resp = await getWiFiSSIDs();
  if (resp === ORBIT_OK) {
    wifiSelect.disabled = false;
    wifiPass.disabled = false;
    wifiConnectBtn.disabled = false;
     availableWiFiSSIDs.forEach(ssid => {
      const option = document.createElement("option");
      option.style.color = "black";
      option.value = ssid;
      option.textContent = ssid;
      wifiSelect.appendChild(option);
    });
  

  wifiConnectBtn.onclick = () => {
    const selectedSSID = wifiSelect.value;
    const password = wifiPass.value.trim();
    
    showConfirm({
          icon: "📶",
          title: "Wi-Fi settings sent!",
          subtitle: `The device will now attempt to connect to "${selectedSSID}". If the connection is successful, you will see updated network information on the dashboard.`,
          onConfirm: () => {
            setSettingsWiFi(selectedSSID, password);
          }
        });

  }


  document.getElementById("settings-name-input").value = systemSettings.user_name;
  document.getElementById("settings-device-name-input").value = systemSettings.device_name;
}}

// ---------- LEAVE PORTAL ----------
document.getElementById("leave-portal-btn").onclick = () => {
  showConfirm({
    icon: "📶",
    title: "Leave portal?",
    subtitle: "You will be disconnected from the Zaire device.",
    onConfirm: () => {
      leavePortal().then(r => {
        if (r === ORBIT_OK) {
          document.body.innerHTML = `
            <div style="color:white;display:flex;justify-content:center;align-items:center;height:100vh;background:#020314;font-family:Arial,sans-serif;text-align:center;padding:20px;">
              <div>
                <h2>Portal Closed</h2>
                <p>You can now return to your normal Wi-Fi connection.</p>
              </div>
            </div>
          `;
        }
      });
    }
  });
};

// ---------- CONFIRM MODAL ----------

let _confirmCallback = null;

function showConfirm({ icon, title, subtitle, onConfirm }) {
  document.getElementById("confirm-icon").textContent = icon || "⚠️";
  document.getElementById("confirm-title").textContent = title || "Are you sure?";
  document.getElementById("confirm-subtitle").textContent = subtitle || "";
  _confirmCallback = onConfirm;
  document.getElementById("confirm-overlay").classList.remove("hidden");
}

function hideConfirm() {
  document.getElementById("confirm-overlay").classList.add("hidden");
  _confirmCallback = null;
}

function initConfirmModal() {
  document.getElementById("confirm-cancel").onclick = hideConfirm;
  document.getElementById("confirm-ok").onclick = () => {
    if (_confirmCallback) _confirmCallback();
    hideConfirm();
  };
  document.getElementById("confirm-overlay").onclick = (e) => {
    if (e.target === document.getElementById("confirm-overlay")) hideConfirm();
  };
}

// ---------- SETTINGS ----------

async function initSettings() {
  const editToggle = document.getElementById("settings-edit-toggle");
  const nameInput = document.getElementById("settings-name-input");
  const deviceNameInput = document.getElementById("settings-device-name-input");
  const deviceLang = document.getElementById("settings-language");
  const unitsSelect = document.getElementById("settings-units");
  const themeSelect = document.getElementById("settings-theme");
  const f1Select = document.getElementById("settings-f1");
  const passcodeToggle = document.getElementById("settings-passcode-toggle");
  const passcodeCard = document.getElementById("settings-passcode-fields");
  const dangerZoneCard = document.getElementById("danger-zone-card");
  const resetDefaultBtn = document.getElementById("settings-restore-defaults-btn");
  const resetFactoryBtn = document.getElementById("settings-factory-reset-btn");
  const saveBtn = document.getElementById("settings-save-btn");

  const manufacturerText = document.getElementById("manufacturer-label");

  var resp;
  resp = await getSettingsUserName();
  systemSettings.user_name = resp;

  resp = await getSettingsDeviceName();
  systemSettings.device_name = resp;

  resp = await getSettingsDeviceLang();
  systemSettings.device_lang = resp;

  resp = await getSettingsDeviceUnit();
  systemSettings.units = resp;

  resp = await getSettingsDeviceTheme();
  systemSettings.theme = resp;

  resp = await getSettingsDeviceF1();
  systemSettings.f1 = resp;

  manufacturerText.innerText = systemInfo.manufacturer;

  
  console.log("Loaded settings:", systemSettings);

  nameInput.value = systemSettings.user_name || "RIDER";
  deviceNameInput.value = systemSettings.device_name || "ZAIRE";
  deviceLang.value = systemSettings.device_lang || "en";
  unitsSelect.value = systemSettings.units || "0";
  themeSelect.value = systemSettings.theme || "0";
  f1Select.value = systemSettings.f1 || "0";
  passcodeToggle.checked = false;
  passcodeCard.classList.add("hidden");

  editToggle.onchange = function () {
    const editing = this.checked;

    nameInput.disabled = !editing;
    deviceNameInput.disabled = !editing;
    deviceLang.disabled = !editing;
    unitsSelect.disabled = !editing;
    themeSelect.disabled = !editing;
    f1Select.disabled = !editing;
    passcodeToggle.disabled = !editing;
    resetDefaultBtn.disabled = !editing;
    resetFactoryBtn.disabled = !editing;
    f1Select.disabled = !editing;

    if (editing) {
      nameInput.classList.add("settings-name-input--active");
      deviceNameInput.classList.add("settings-name-input--active");
      nameInput.focus();
      saveBtn.classList.remove("hidden");
      dangerZoneCard.classList.remove("hidden");
    } else {
      nameInput.classList.remove("settings-name-input--active");
      deviceNameInput.classList.remove("settings-name-input--active");
      passcodeToggle.checked = false;
      passcodeCard.classList.add("hidden");
      saveBtn.classList.add("hidden");
      dangerZoneCard.classList.add("hidden");
    }

    passcodeToggle.onchange = function () {
      passcodeCard.classList.toggle("hidden", !this.checked);
    };
  };

  resetDefaultBtn.onclick = () => {
    showConfirm({
      icon: "⚠️",
      title: "Restore default settings?",
      subtitle: "This will reset all settings to their default values, except for your username and device name.",
      onConfirm: () => {
        console.log("Restoring defaults...");
      }
    });
  };

  resetFactoryBtn.onclick = () => {
    showConfirm({
      icon: "⚠️",
      title: "Factory reset?",
      subtitle: "This will reset all settings to their default values.",
      onConfirm: () => {
        console.log("Performing factory reset...");
      }
    });
  };

  saveBtn.onclick = () => {
    const newName = nameInput.value.trim().toUpperCase();
    const newDeviceName = deviceNameInput.value.trim().toUpperCase();
    const newLanguage = deviceLang.value;
    const newUnits = unitsSelect.value;
    const newTheme = themeSelect.value;

    if (!newName || !newDeviceName) return;

    systemSettings.user_name = newName;
    systemSettings.device_name = newDeviceName;
    systemSettings.device_lang = newLanguage;
    systemSettings.units = newUnits;
    systemSettings.theme = newTheme;

    document.getElementById("dash-username").textContent = newName;
    document.querySelector("#dash-avatar span").textContent = newName[0];
    document.getElementById("dash-subtitle").textContent = newDeviceName;

    nameInput.value = newName;
    deviceNameInput.value = newDeviceName;

    editToggle.checked = false;
    nameInput.disabled = true;
    deviceNameInput.disabled = true;
    deviceLang.disabled = true;
    unitsSelect.disabled = true;
    themeSelect.disabled = true;
    f1Select.disabled = true;
    passcodeToggle.checked = false;
    passcodeToggle.disabled = true;
    resetDefaultBtn.disabled = true;
    resetFactoryBtn.disabled = true;
    nameInput.classList.remove("settings-name-input--active");
    deviceNameInput.classList.remove("settings-name-input--active");
    passcodeCard.classList.add("hidden");
    saveBtn.classList.add("hidden");
    dangerZoneCard.classList.add("hidden");

    saveAllSettings();
  };
}

// ---------- BOOT ----------

document.addEventListener("DOMContentLoaded", () => {

  renderDashboard();
  initConfirmModal();
  initSettings();

  document.getElementById("nav-dashboard").onclick = () =>
    showView("dashboard-view", document.getElementById("nav-dashboard"));

  document.getElementById("nav-settings").onclick = () =>
    showView("settings-view", document.getElementById("nav-settings"));

});