// --------------------------------------------
// THIS WAS A FULL COPY OF 'ZAIRE SYSTEM' ANOTHER PROJECT I WORKED ON. 
// I COPIED THIS FILE AS A STARTING POINT FOR THE SETTINGS STRUCTURE AND FUNCTIONS, 
// AND THEN MODIFIED IT FOR THE ORBIT SYSTEM. 
// THERE MAY BE SOME UNUSED OR OUTDATED CODE IN THIS FILE, 
// BUT I LEFT IT IN FOR NOW AS A REFERENCE FOR ANY FUTURE SETTINGS I MAY WANT TO ADD.
// --------------------------------------------

//ORBIT SYSTEMS
const ORBIT_OK = "ORBIT_OK"
const ORBIT_ERR = "ORBIT_ERR"
// CODEx structure for settings updates from web portal to ESP32. The web portal will send a codex key along with the new value, and the ESP32 will know which setting to update based on the codex key.
const CODEX = Object.freeze({
  FIRST_BOOT: 0x0A,
  USER_NAME: 0x0B,
  DEVICE_NAME: 0x0C,
  DEVICE_LANG: 0x0D,
  DEVICE_UNITS: 0x0E,
  DEVICE_THEME: 0x0F,
  DEVICE_F1: 0x12,
  PASSCODE: 0x10,
  RECOVERY_CODE: 0x11,

  FIRMWARE_VERSION: 0xA0,
  BUILD_DATE: 0xA1,
  DEVICE_ID: 0xA2,
  HARDWARE_REV: 0xA3,
  BLUETOOTH_VERSION: 0xA4,
  MANUFACTURER: 0xB5,
  MODEL_NAME: 0xB6,

  ADV_WIFIS: 0xC0, //used to send available wifi SSIDs to portal for display
  CON_WIFI: 0xC1, //used to send selected wifi SSID and password for connection
});

//SYSTEM SETTINGS
var systemSettings = {//S
  first_boot: 0,
  user_name: null,
  device_name: null, 
  device_lang: null,
  units: null,
  theme: null,
  f1: null,

  passcode: null,
  recovery_code: null,
};

var systemInfo = {
  manufacturer: "ORBIT SYSTEMS",
  model_name: null,
  firmware_version: null,
  build_date: null,
  device_id: null,
  hardware_rev: null,
  bluetooth_version: null
};


//SET SETTINGS
async function setSettingsFirstBoot() {
  return await UpdateSettings(`&${CODEX.FIRST_BOOT}&1&`);
}
async function setSettingsUserName(val) {
  return await UpdateSettings(`&${CODEX.USER_NAME}&${val}&`);
}

async function setSettingsDeviceName(val) {
  return await UpdateSettings(`&${CODEX.DEVICE_NAME}&${val}&`);
}

async function setSettingsDeviceLang(val) {
  return await UpdateSettings(`&${CODEX.DEVICE_LANG}&${val}&`);
}

async function setSettingsDeviceUnit(val) {
  return await UpdateSettings(`&${CODEX.DEVICE_UNITS}&${val}&`);
}

async function setSettingsDeviceTheme(val) {
  return await UpdateSettings(`&${CODEX.DEVICE_THEME}&${val}&`);
}

async function setSettingsPassCode(val) {
  return await UpdateSettings(`&${CODEX.PASSCODE}&${val}&`);
}

async function setSettingsRecoveryCode(val) {
  return await UpdateSettings(`&${CODEX.RECOVERY_CODE}&${val}&`);
}

async function setSettingsDeviceF1(val) {
  return await UpdateSettings(`&${CODEX.DEVICE_F1}&${val}&`);
}

async function setSettingsWiFi(vwifi, pwd) {
  return await UpdateSettings(`&${CODEX.CON_WIFI}&${vwifi}&${pwd}&`);
}

//GET SETTINGS
async function getSettingsFirstBoot() {
  return await LoadSettings(`&${CODEX.FIRST_BOOT}&1&`);
}
async function getSettingsUserName (val) {
  return await LoadSettings(`&${CODEX.USER_NAME}&${val}&`);
}
async function getSettingsDeviceName (val) {
  return await LoadSettings(`&${CODEX.DEVICE_NAME}&${val}&`);
}
async function getSettingsDeviceLang (val) {
  return await LoadSettings(`&${CODEX.DEVICE_LANG}&${val}&`);
}
async function getSettingsDeviceUnit (val) {
  return await LoadSettings(`&${CODEX.DEVICE_UNITS}&${val}&`);
}
async function getSettingsDeviceTheme (val) {
  return await LoadSettings(`&${CODEX.DEVICE_THEME}&${val}&`);
}

async function getSettingsDeviceF1 (val) {
  return await LoadSettings(`&${CODEX.DEVICE_F1}&${val}&`);
}

async function getSettingsPassCode (val) {
  return await LoadSettings(`&${CODEX.PASSCODE}&${val}&`);
}

async function getSystemInfoManufacturer() {
  return await LoadSettings(`&${CODEX.MANUFACTURER}&1&`);
}

async function getSystemInfoModelName() {
  return await LoadSettings(`&${CODEX.MODEL_NAME}&1&`);
}

async function getSystemInfoFirmwareVersion() {
  return await LoadSettings(`&${CODEX.FIRMWARE_VERSION}&1&`);
}

async function getSystemInfoBuildDate() {
  return await LoadSettings(`&${CODEX.BUILD_DATE}&1&`);
}

async function getSystemInfoDeviceID() {
  return await LoadSettings(`&${CODEX.DEVICE_ID}&1&`);
}

async function getSystemInfoHardwareRev() {
  return await LoadSettings(`&${CODEX.HARDWARE_REV}&1&`);
}

async function getSystemInfoBluetoothVersion() {
  return await LoadSettings(`&${CODEX.BLUETOOTH_VERSION}&1&`);
}

//SAVE SETTINGS

//LEAVE PORTAL
async function leavePortal() {
  try {
    const response = await fetch('/settings_leave_portal', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',   // or 'text/plain' if you prefer
      },
      body: JSON.stringify("none")      // Send your data here
      // If you want to send plain text instead:
      // body: settingsData.toString()
    });

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }



    const responseText = await response.text();   // ESP32 should reply with text
    return responseText;

  } catch (err) {
    console.error("Failed to communicate with ESP32:", err);
    throw err;   // rethrow if you want the caller to handle the error
  }
}

async function setOnBoardSettings() {

  var resp;
  resp = await setSettingsUserName(systemSettings.user_name);
  console.log(systemSettings.user_name);
  if(resp != ORBIT_OK)return ORBIT_ERR;

  resp = await setSettingsDeviceName(systemSettings.device_name);  
  if(resp != ORBIT_OK)return ORBIT_ERR;

  resp = await setSettingsDeviceLang(systemSettings.device_lang);
  if(resp != ORBIT_OK)return ORBIT_ERR;

  resp = await setSettingsDeviceUnit(systemSettings.units);
  if(resp != ORBIT_OK)return ORBIT_ERR;
  
  resp = await setSettingsDeviceTheme(systemSettings.theme);
  if(resp != ORBIT_OK)return ORBIT_ERR;

  resp = await setSettingsDeviceF1(systemSettings.f1);
  if(resp != ORBIT_OK)return ORBIT_ERR;


  resp = await setSettingsPassCode(systemSettings.passcode);
  if(resp != ORBIT_OK)return ORBIT_ERR;
  
  
  // resp = await setSettingsRecoveryCode(systemSettings.recovery_code);
  // if(resp != ORBIT_OK)return ORBIT_ERR;
  // 


  resp = await setSettingsFirstBoot();
  if(resp != ORBIT_OK)return ORBIT_ERR;

  return ORBIT_OK;
}

async function getStartUpSettings(){
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

  console.log("Settings loaded from ESP32:", systemSettings);

  return ORBIT_OK;
}

async function saveAllSettings() {
  var resp;

  console.log(systemSettings.user_name);
  resp = await setSettingsUserName(systemSettings.user_name);
  resp != ORBIT_OK? console.error("Failed to save user name") : console.log("User name saved");

  resp = await setSettingsDeviceName(systemSettings.device_name);
  resp != ORBIT_OK? console.error("Failed to save device name") : console.log("Device name saved");

  resp = await setSettingsDeviceLang(systemSettings.device_lang);
  resp != ORBIT_OK? console.error("Failed to save device language") : console.log("Device language saved");

  resp = await setSettingsDeviceUnit(systemSettings.units);
  resp != ORBIT_OK? console.error("Failed to save device units") : console.log("Device units saved");

  resp = await setSettingsDeviceTheme(systemSettings.theme);
  resp != ORBIT_OK? console.error("Failed to save device theme") : console.log("Device theme saved");

}

async function UpdateSettings(settingsData) {
  try {
    const response = await fetch('/settings_set_settings_data', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',   // or 'text/plain' if you prefer
      },
      body: JSON.stringify(settingsData)      // Send your data here
      // If you want to send plain text instead:
      // body: settingsData.toString()
    });

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    const responseText = await response.text();   // ESP32 should reply with text
    return responseText;

  } catch (err) {
    console.error("Failed to communicate with ESP32:", err);
    throw err;   // rethrow if you want the caller to handle the error
  }
}

//LOADS settings
async function LoadSettings(settingsData){
  
  try {
    const response = await fetch('/settings_get_settings_data', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',   // or 'text/plain' if you prefer
      },
      body: JSON.stringify(settingsData)      // Send your data here
      // If you want to send plain text instead:
      // body: settingsData.toString()
    });

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    const responseText = await response.text();   // ESP32 should reply with text
    
    console.log("ESP32 replied:", responseText);

    return responseText;

  } catch (err) {
    console.error("Failed to communicate with ESP32:", err);
    throw err;   // rethrow if you want the caller to handle the error
  }

}

//<-------------- SYSTEM GRAB WIFIs ------------->//


let availableWiFiSSIDs = [];

async function getWiFiSSIDs(){

  try {
    const response = await fetch('/settings_get_settings_data', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',   // or 'text/plain' if you prefer
      },
      body: JSON.stringify(`&${CODEX.ADV_WIFIS}&1&`)      // Send your data here)
    });

    if (!response.ok) {
      throw new Error(`HTTP error! Status: ${response.status}`);
    }

    const responseText = await response.text();   // ESP32 should reply with text
    
    console.log("ESP32 replied:", responseText);

    const wifiData = responseText.split(','); // Assuming the ESP32 sends a comma-separated list of SSIDs
    availableWiFiSSIDs = wifiData.map(ssid => ssid.trim()); // Trim any whitespace

    console.log("Available Wi-Fi SSIDs:", availableWiFiSSIDs);

    return ORBIT_OK;

  } catch (err) {
    console.error("Failed to communicate with ESP32:", err);
    throw err;   // rethrow if you want the caller to handle the error
  }

}

//<-------------- GLOBAL LANGUAGE ------------->//
// --------------------------------------------
// I18N TRANSLATION TABLE
// --------------------------------------------
const i18n = {
  en: {
    main_title: "Let’s get your device ready.",
    lang_title: "Choose your language.",
    lang_subtitle: "We’ll use this across your device experience.",

    name_title: "What should we call you?",
    name_subtitle: "This name will appear in your device settings.",
    name_placeholder: "Your name",

    device_title: "Name your device.",
    device_subtitle: "You’ll see this name when connecting to the device.",

    units_title: "Choose your team.",
    units_subtitle: "We’ll use this for F1 showcase",
    units_metric_label: "MCLAREN",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "FERRARI",
    units_imperial_note: "(mph, °F)",

    theme_title: "Pick your screen theme.",
    theme_subtitle: "You can change this later in settings.",
    theme_dark_label: "Dark",
    theme_light_label: "Light",

    passcode_title: "Create a passcode.",
    passcode_subtitle: "Choose a 6-digit code to unlock your device.",
    passcode_confirm_placeholder: "Confirm ******",

    recovery_title: "Recovery number.",
    recovery_subtitle:
      "Choose a single number from 0–9.\nWe’ll use it to help recover your passcode.",

    login_title: "Enter Passcode",
    login_subtitle: "Enter your 6-digit code to continue.",

    btn_lang_continue: "Continue",
    btn_continue: "Continue",
    btn_finish: "Finish setup",
    btn_unlock: "UNLOCK",
    btn_back: "Back",

    saving_title: "Setting things up…",
    saving_subtitle: "Saving your preferences and preparing your ORBIT interface.",

  },

  es: {
    main_title: "Preparemos tus gafas.",
    lang_title: "Elige tu idioma.",
    lang_subtitle: "Lo usaremos en toda la experiencia de tus gafas.",

    name_title: "¿Cómo debemos llamarte?",
    name_subtitle: "Este nombre aparecerá en la configuración de tus gafas.",
    name_placeholder: "Tu nombre",

    device_title: "Nombra tus gafas.",
    device_subtitle: "Verás este nombre al conectarte al dispositivo.",

    units_title: "Elige tus unidades.",
    units_subtitle: "Usaremos esto para velocidad, temperatura y altitud.",
    units_metric_label: "Métrico",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "Imperial",
    units_imperial_note: "(mph, °F)",

    theme_title: "Elige el tema de pantalla.",
    theme_subtitle: "Puedes cambiarlo más tarde en configuración.",
    theme_dark_label: "Oscuro",
    theme_light_label: "Claro",

    passcode_title: "Crea un código de acceso.",
    passcode_subtitle: "Elige un código de 6 dígitos para desbloquear tus gafas.",
    passcode_confirm_placeholder: "Confirmar ******",

    recovery_title: "Número de recuperación.",
    recovery_subtitle:
      "Elige un solo número del 0 al 9.\nLo usaremos para ayudarte a recuperar tu código.",

    login_title: "Introduce el código",
    login_subtitle: "Introduce tu código de 6 dígitos para continuar.",

    btn_lang_continue: "Continuar",
    btn_continue: "Continuar",
    btn_finish: "Finalizar",
    btn_unlock: "DESBLOQUEAR",
    btn_back: "Atrás",

    saving_title: "Configurando todo…",
    saving_subtitle: "Guardando tus preferencias y preparando la interfaz de ORBIT.",

  },

  fr: {
    main_title: "Préparons vos lunettes.",
    lang_title: "Choisissez votre langue.",
    lang_subtitle: "Nous l’utiliserons dans toute l’expérience de vos lunettes.",

    name_title: "Comment devons-nous vous appeler ?",
    name_subtitle: "Ce nom apparaîtra dans les réglages de vos lunettes.",
    name_placeholder: "Votre nom",

    device_title: "Nommez vos lunettes.",
    device_subtitle: "Ce nom s’affichera lors de la connexion au dispositif.",

    units_title: "Choisissez vos unités.",
    units_subtitle:
      "Elles seront utilisées pour la vitesse, la température et l’altitude.",
    units_metric_label: "Métrique",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "Impérial",
    units_imperial_note: "(mph, °F)",

    theme_title: "Choisissez un thème d’écran.",
    theme_subtitle: "Vous pourrez le modifier plus tard dans les réglages.",
    theme_dark_label: "Sombre",
    theme_light_label: "Clair",

    passcode_title: "Créez un code d’accès.",
    passcode_subtitle:
      "Choisissez un code à 6 chiffres pour déverrouiller vos lunettes.",
    passcode_confirm_placeholder: "Confirmer ******",

    recovery_title: "Numéro de récupération.",
    recovery_subtitle:
      "Choisissez un seul chiffre entre 0 et 9.\nNous l’utiliserons pour récupérer votre code.",

    login_title: "Entrez le code",
    login_subtitle:
      "Saisissez votre code à 6 chiffres pour continuer.",

    btn_lang_continue: "Continuer",
    btn_continue: "Continuer",
    btn_finish: "Terminer",
    btn_unlock: "DÉVERROUILLER",
    btn_back: "Retour",

    saving_title: "Nous préparons tout…",
    saving_subtitle: "Enregistrement de vos préférences et préparation de l’interface ORBIT.",
  },

  cn: {
    main_title: "让我们开始设置你的护目镜。",
    lang_title: "选择你的语言。",
    lang_subtitle: "我们会在整个护目镜体验中使用此语言。",

    name_title: "我们该怎么称呼你？",
    name_subtitle: "此名称将显示在护目镜的设置中。",
    name_placeholder: "你的名字",

    device_title: "为你的护目镜命名。",
    device_subtitle: "连接设备时将显示此名称。",

    units_title: "选择你的单位制。",
    units_subtitle: "我们会将其用于速度、温度和海拔。",
    units_metric_label: "公制",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "英制",
    units_imperial_note: "(mph, °F)",

    theme_title: "选择你的屏幕主题。",
    theme_subtitle: "稍后可以在设置中更改。",
    theme_dark_label: "深色",
    theme_light_label: "浅色",

    passcode_title: "创建访问密码。",
    passcode_subtitle: "选择一个 6 位数字用于解锁你的护目镜。",
    passcode_confirm_placeholder: "确认 ******",

    recovery_title: "恢复号码。",
    recovery_subtitle:
      "从 0 到 9 中选择一个数字。\n我们将使用它来帮助你找回密码。",

    login_title: "输入密码",
    login_subtitle: "输入你的 6 位密码以继续。",

    btn_lang_continue: "继续",
    btn_continue: "继续",
    btn_finish: "完成设置",
    btn_unlock: "解锁",
    btn_back: "返回",

    saving_title: "正在准备系统…",
    saving_subtitle: "正在保存您的偏好并准备 ORBIT 界面。",

  },

  hi: {
    main_title: "आइए आपकी गॉगल्स तैयार करें।",
    lang_title: "अपनी भाषा चुनें।",
    lang_subtitle: "हम इसे आपकी पूरी गॉगल्स अनुभव में उपयोग करेंगे।",

    name_title: "हम आपको क्या कहकर बुलाएँ?",
    name_subtitle: "यह नाम आपकी गॉगल्स की सेटिंग में दिखाई देगा।",
    name_placeholder: "आपका नाम",

    device_title: "अपनी गॉगल्स को नाम दें।",
    device_subtitle: "डिवाइस से कनेक्ट करते समय यह नाम दिखाई देगा।",

    units_title: "अपनी इकाइयाँ चुनें।",
    units_subtitle:
      "हम इसे गति, तापमान और ऊँचाई के लिए उपयोग करेंगे।",
    units_metric_label: "मीट्रिक",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "इंपीरियल",
    units_imperial_note: "(mph, °F)",

    theme_title: "अपना स्क्रीन थीम चुनें।",
    theme_subtitle: "आप इसे बाद में सेटिंग में बदल सकते हैं।",
    theme_dark_label: "डार्क",
    theme_light_label: "लाइट",

    passcode_title: "पासकोड बनाएँ।",
    passcode_subtitle:
      "अपनी गॉगल्स को अनलॉक करने के लिए 6-अंकों का कोड चुनें।",
    passcode_confirm_placeholder: "पुष्टि करें ******",

    recovery_title: "रिकवरी नंबर।",
    recovery_subtitle:
      "0 से 9 तक एक संख्या चुनें।\nहम इसका उपयोग पासकोड पुनर्प्राप्ति में मदद के लिए करेंगे।",

    login_title: "पासकोड दर्ज करें",
    login_subtitle: "जारी रखने के लिए 6-अंकों का कोड दर्ज करें।",

    btn_lang_continue: "जारी रखें",
    btn_continue: "जारी रखें",
    btn_finish: "सेटअप पूरा करें",
    btn_unlock: "अनलॉक",
    btn_back: "वापस",

    saving_title: "सब कुछ तैयार किया जा रहा है…",
    saving_subtitle: "आपकी पसंद सहेजी जा रही हैं और ORBIT इंटरफ़ेस तैयार किया जा रहा है।",
  },

  jp: {
    main_title: "ゴーグルの準備をしましょう。",
    lang_title: "言語を選択してください。",
    lang_subtitle: "この言語はゴーグル全体の体験に使用されます。",

    name_title: "あなたを何と呼べば良いですか？",
    name_subtitle: "この名前はゴーグルの設定に表示されます。",
    name_placeholder: "あなたの名前",

    device_title: "ゴーグルに名前を付けてください。",
    device_subtitle: "デバイスに接続するときにこの名前が表示されます。",

    units_title: "単位を選択してください。",
    units_subtitle: "速度、温度、高度に使用します。",
    units_metric_label: "メートル法",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "ヤード・ポンド法",
    units_imperial_note: "(mph, °F)",

    theme_title: "画面テーマを選択してください。",
    theme_subtitle: "後から設定で変更できます。",
    theme_dark_label: "ダーク",
    theme_light_label: "ライト",

    passcode_title: "パスコードを作成してください。",
    passcode_subtitle: "ゴーグルを解除するための6桁のコードを選択してください。",
    passcode_confirm_placeholder: "確認 ******",

    recovery_title: "リカバリ番号。",
    recovery_subtitle:
      "0〜9の数字を1つ選んでください。\nパスコードの復旧に使用します。",

    login_title: "パスコードを入力",
    login_subtitle: "続行するには6桁のコードを入力してください。",

    btn_lang_continue: "続行",
    btn_continue: "続行",
    btn_finish: "設定完了",
    btn_unlock: "アンロック",
    btn_back: "戻る",

    saving_title: "設定を適用しています…",
    saving_subtitle: "設定を保存し、ORBIT インターフェースを準備しています。",

  },

  kr: {
    main_title: "고글을 준비해볼까요?",
    lang_title: "언어를 선택하세요.",
    lang_subtitle: "이 언어는 전체 고글 경험에 사용됩니다.",

    name_title: "어떻게 불러드릴까요?",
    name_subtitle: "이 이름은 고글 설정에 표시됩니다.",
    name_placeholder: "이름 입력",

    device_title: "고글 이름을 지정하세요.",
    device_subtitle: "장치에 연결할 때 이 이름이 표시됩니다。",

    units_title: "단위를 선택하세요.",
    units_subtitle: "속도, 온도, 고도 측정에 사용됩니다.",
    units_metric_label: "미터법",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "야드파운드법",
    units_imperial_note: "(mph, °F)",

    theme_title: "화면 테마를 선택하세요。",
    theme_subtitle: "나중에 설정에서 변경할 수 있습니다。",
    theme_dark_label: "다크",
    theme_light_label: "라이트",

    passcode_title: "암호를 생성하세요。",
    passcode_subtitle: "고글 잠금을 해제할 6자리 숫자를 선택하세요。",
    passcode_confirm_placeholder: "확인 ******",

    recovery_title: "복구 번호。",
    recovery_subtitle:
      "0–9 사이에서 숫자 하나를 선택하세요。\n암호 복구 시 사용됩니다。",

    login_title: "암호 입력",
    login_subtitle: "계속하려면 6자리 코드를 입력하세요。",

    btn_lang_continue: "계속",
    btn_continue: "계속",
    btn_finish: "설정 완료",
    btn_unlock: "잠금 해제",
    btn_back: "뒤로",

    saving_title: "설정을 적용하는 중…",
    saving_subtitle: "선호 설정을 저장하고 ORBIT 인터페이스를 준비하고 있습니다.",

  },

  pt: {
    main_title: "Vamos preparar seus óculos.",
    lang_title: "Escolha seu idioma.",
    lang_subtitle: "Usaremos isso em toda a experiência dos seus óculos.",

    name_title: "Como devemos te chamar?",
    name_subtitle: "Esse nome aparecerá nas configurações dos seus óculos.",
    name_placeholder: "Seu nome",

    device_title: "Dê um nome aos seus óculos.",
    device_subtitle: "Você verá esse nome ao conectar ao dispositivo.",

    units_title: "Escolha suas unidades.",
    units_subtitle:
      "Usaremos isso para velocidade, temperatura e altitude.",
    units_metric_label: "Métrico",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "Imperial",
    units_imperial_note: "(mph, °F)",

    theme_title: "Escolha o tema da tela.",
    theme_subtitle: "Você pode alterá-lo mais tarde nas configurações.",
    theme_dark_label: "Escuro",
    theme_light_label: "Claro",

    passcode_title: "Crie um código de acesso.",
    passcode_subtitle:
      "Escolha um código de 6 dígitos para desbloquear seus óculos.",
    passcode_confirm_placeholder: "Confirmar ******",

    recovery_title: "Número de recuperação.",
    recovery_subtitle:
      "Escolha um número de 0 a 9.\nUsaremos isso para ajudar a recuperar seu código.",

    login_title: "Insira o código",
    login_subtitle: "Insira seu código de 6 dígitos para continuar.",

    btn_lang_continue: "Continuar",
    btn_continue: "Continuar",
    btn_finish: "Finalizar",
    btn_unlock: "DESBLOQUEAR",
    btn_back: "Voltar",

    saving_title: "Configurando tudo…",
    saving_subtitle: "Salvando suas preferências e preparando a interface do ORBIT.",

  },

  ru: {
    main_title: "Давайте подготовим ваши очки.",
    lang_title: "Выберите язык.",
    lang_subtitle: "Он будет использоваться во всём интерфейсе ваших очков.",

    name_title: "Как нам к вам обращаться?",
    name_subtitle: "Это имя появится в настройках ваших очков.",
    name_placeholder: "Ваше имя",

    device_title: "Назовите ваши очки.",
    device_subtitle: "Это имя будет отображаться при подключении к устройству.",

    units_title: "Выберите единицы измерения.",
    units_subtitle:
      "Мы будем использовать их для скорости, температуры и высоты.",
    units_metric_label: "Метрическая",
    units_metric_note: "(км/ч, °C)",
    units_imperial_label: "Имперская",
    units_imperial_note: "(миль/ч, °F)",

    theme_title: "Выберите тему интерфейса.",
    theme_subtitle:
      "Вы сможете изменить её позже в настройках.",
    theme_dark_label: "Тёмная",
    theme_light_label: "Светлая",

    passcode_title: "Создайте пароль.",
    passcode_subtitle:
      "Выберите 6-значный код для разблокировки очков.",
    passcode_confirm_placeholder: "Подтвердить ******",

    recovery_title: "Резервный номер.",
    recovery_subtitle:
      "Выберите одну цифру от 0 до 9.\nМы используем её для восстановления пароля.",

    login_title: "Введите пароль",
    login_subtitle:
      "Введите 6-значный код, чтобы продолжить.",

    btn_lang_continue: "Продолжить",
    btn_continue: "Продолжить",
    btn_finish: "Завершить",
    btn_unlock: "РАЗБЛОКИРОВАТЬ",
    btn_back: "Назад",

    saving_title: "Выполняется настройка…",
    saving_subtitle: "Сохраняем ваши параметры и подготавливаем интерфейс ORBIT.",

  },

  ar: {
    main_title: "لنُجهّز نظارتك الآن.",
    lang_title: "اختر لغتك.",
    lang_subtitle: "سنستخدم هذه اللغة في جميع واجهات نظارتك.",

    name_title: "بماذا نُناديك؟",
    name_subtitle: "سيظهر هذا الاسم في إعدادات نظارتك.",
    name_placeholder: "اسمك",

    device_title: "قُم بتسمية نظارتك.",
    device_subtitle: "سيظهر هذا الاسم عند الاتصال بالجهاز.",

    units_title: "اختر وحداتك.",
    units_subtitle: "سنستخدمها للسرعة والحرارة والارتفاع.",
    units_metric_label: "المتري",
    units_metric_note: "(km/h, °C)",
    units_imperial_label: "الإمبريالي",
    units_imperial_note: "(mph, °F)",

    theme_title: "اختر سمة الشاشة.",
    theme_subtitle: "يمكنك تغييرها لاحقًا من الإعدادات.",
    theme_dark_label: "داكن",
    theme_light_label: "فاتح",

    passcode_title: "أنشئ رمز مرور.",
    passcode_subtitle:
      "اختر رمزًا مكوّنًا من 6 أرقام لفتح نظارتك.",
    passcode_confirm_placeholder: "تأكيد ******",

    recovery_title: "رقم الاستعادة.",
    recovery_subtitle:
      "اختر رقمًا واحدًا من 0 إلى 9.\nسنستخدمه للمساعدة في استعادة رمز المرور.",

    login_title: "أدخل رمز المرور",
    login_subtitle: "أدخل رمزك المكوّن من 6 أرقام للمتابعة.",

    btn_lang_continue: "استمرار",
    btn_continue: "استمرار",
    btn_finish: "إنهاء الإعداد",
    btn_unlock: "فتح",
    btn_back: "رجوع",

    saving_title: "جارٍ إعداد النظام…",
    saving_subtitle: "يتم حفظ تفضيلاتك وتحضير واجهة ORBIT.",

  }
};

//<--------------------- END --------------------->//