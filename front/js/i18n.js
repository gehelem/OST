// js/i18n.js — tiny local dictionary for UI3-only strings: labels this
// client invents itself (panel titles, checkbox labels, tooltips, status
// words, empty-state text...) that the backend never sends at all, because
// they don't exist server-side — as opposed to property/element labels,
// which DO come translated from the server already (see
// documentation/content/dev/dev-client/*.md) and must never be duplicated
// here.
//
// Gilles, 2026-09-30, having noticed "True horizon" (the sky dome's own
// settings panel) stayed in English regardless of the selected language:
// "tu mets en place pas mal de libellés spécifiques (qui sont locaux à
// l'appli et pas gérés par le backend) ... ajoute donc une mécanique avec
// un dictionnaire de traduction local dédié, qui pointe sur le même
// language que celui selectionné pour le backend".
//
// Deliberately NOT a general-purpose i18n framework — no pluralization, no
// ICU message syntax, no ${} interpolation beyond simple concatenation at
// the call site. Just enough to keep local-only strings from being
// silently English-only while everything server-sourced already follows
// the selected language.
//
// Language tracking: this module has no connection of its own to read
// from - it mirrors core/connection.js's own `language` field (the single
// source of truth already used for DU/IL/LO), kept in sync by app.js's two
// call sites that ever change it (the initial connect() and the
// lang-select's own change handler). See setLanguage()/getLanguage().

const DEFAULT_LANG = "en";
let currentLang = DEFAULT_LANG;

export function setLanguage(lang) {
  currentLang = lang || DEFAULT_LANG;
}

export function getLanguage() {
  return currentLang;
}

/** Every UI3-local string lives here, one entry per supported language
 * (same 5 as the lang-select: en/fr/de/it/es). Keys are short dotted ids
 * grouped by where they're used, not the English text itself, so a key
 * never silently goes stale if the English wording changes later. */
const STRINGS = {
  // ---- shared across several templates/panels ----------------------------
  "common.other": { en: "Other", fr: "Autre", de: "Sonstiges", it: "Altro", es: "Otro" },
  "common.moduleNotFound": { en: "Module not found (unloaded?)", fr: "Module introuvable (déchargé ?)", de: "Modul nicht gefunden (entladen?)", it: "Modulo non trovato (scaricato?)", es: "Módulo no encontrado (¿descargado?)" },
  "common.general": { en: "General", fr: "Général", de: "Allgemein", it: "Generale", es: "General" },
  "common.close": { en: "Close", fr: "Fermer", de: "Schließen", it: "Chiudi", es: "Cerrar" },
  "common.editRow": { en: "Edit row", fr: "Modifier la ligne", de: "Zeile bearbeiten", it: "Modifica riga", es: "Editar fila" },
  "common.moveUp": { en: "Move up", fr: "Monter", de: "Nach oben", it: "Sposta su", es: "Subir" },
  "common.moveDown": { en: "Move down", fr: "Descendre", de: "Nach unten", it: "Sposta giù", es: "Bajar" },
  "common.deleteRow": { en: "Delete row", fr: "Supprimer la ligne", de: "Zeile löschen", it: "Elimina riga", es: "Eliminar fila" },
  "common.deleteRowConfirm": { en: "Delete this row?", fr: "Supprimer cette ligne ?", de: "Diese Zeile löschen?", it: "Eliminare questa riga?", es: "¿Eliminar esta fila?" },
  "common.sendNow": { en: "Send now (SV)", fr: "Envoyer maintenant (SV)", de: "Jetzt senden (SV)", it: "Invia ora (SV)", es: "Enviar ahora (SV)" },
  "common.noImage": { en: "No image", fr: "Aucune image", de: "Kein Bild", it: "Nessuna immagine", es: "Sin imagen" },
  "common.noVideo": { en: "No video", fr: "Aucune vidéo", de: "Kein Video", it: "Nessun video", es: "Sin vídeo" },
  "common.profile": { en: "Profile", fr: "Profil", de: "Profil", it: "Profilo", es: "Perfil" },
  "common.profileNamePlaceholder": { en: "profile name", fr: "nom du profil", de: "Profilname", it: "nome profilo", es: "nombre del perfil" },

  // ---- INDI-style property/element status words (dom.js) -----------------
  "status.standby": { en: "Standby", fr: "En attente", de: "Bereit", it: "Standby", es: "En espera" },
  "status.ok": { en: "OK", fr: "OK", de: "OK", it: "OK", es: "OK" },
  "status.busy": { en: "Busy", fr: "Occupé", de: "Beschäftigt", it: "Occupato", es: "Ocupado" },
  "status.error": { en: "Error", fr: "Erreur", de: "Fehler", it: "Errore", es: "Error" },
  "status.connecting": { en: "Connecting", fr: "Connexion en cours", de: "Verbinde", it: "Connessione in corso", es: "Conectando" },
  "status.connected": { en: "Connected", fr: "Connecté", de: "Verbunden", it: "Connesso", es: "Conectado" },
  "status.disconnected": { en: "Disconnected", fr: "Déconnecté", de: "Getrennt", it: "Disconnesso", es: "Desconectado" },

  // ---- app chrome: connect bar, top-right icon buttons (app.js/index.html) -
  "chrome.tidyUpPanels": { en: "Tidy up panels", fr: "Réorganiser les fenêtres", de: "Fenster aufräumen", it: "Riordina i pannelli", es: "Ordenar los paneles" },
  "chrome.parameters": { en: "Parameters", fr: "Paramètres", de: "Parameter", it: "Parametri", es: "Parámetros" },
  "chrome.uiSize": { en: "UI size", fr: "Taille de l'interface", de: "Oberflächengröße", it: "Dimensione interfaccia", es: "Tamaño de la interfaz" },
  "chrome.toggleTheme": { en: "Toggle theme", fr: "Changer de thème", de: "Design umschalten", it: "Cambia tema", es: "Cambiar tema" },
  "chrome.connect": { en: "Connect", fr: "Connecter", de: "Verbinden", it: "Connetti", es: "Conectar" },
  "chrome.disconnect": { en: "Disconnect", fr: "Déconnecter", de: "Trennen", it: "Disconnetti", es: "Desconectar" },
  "chrome.minimize": { en: "Minimize", fr: "Réduire", de: "Minimieren", it: "Riduci a icona", es: "Minimizar" },
  "chrome.resize": { en: "Resize", fr: "Redimensionner", de: "Größe ändern", it: "Ridimensiona", es: "Redimensionar" },
  "chrome.parametersPrefix": { en: "Parameters — ", fr: "Paramètres — ", de: "Parameter — ", it: "Parametri — ", es: "Parámetros — " },
  "chrome.notConnectedLede": { en: "Not connected. Set the WebSocket URL above and press Connect.", fr: "Non connecté. Renseignez l'URL WebSocket ci-dessus et cliquez sur Connecter.", de: "Nicht verbunden. Geben Sie oben die WebSocket-URL ein und klicken Sie auf Verbinden.", it: "Non connesso. Imposta l'URL WebSocket sopra e premi Connetti.", es: "No conectado. Indique la URL WebSocket arriba y pulse Conectar." },
  "chrome.recentServers": { en: "Recent servers", fr: "Serveurs récents", de: "Letzte Server", it: "Server recenti", es: "Servidores recientes" },
  "chrome.noHistoryYet": { en: "No history yet.", fr: "Aucun historique pour l'instant.", de: "Noch kein Verlauf.", it: "Ancora nessuna cronologia.", es: "Aún no hay historial." },

  // ---- imagelayer.js: shared image/stats block ----------------------------
  "imagelayer.dimensions": { en: "Dimensions", fr: "Dimensions", de: "Abmessungen", it: "Dimensioni", es: "Dimensiones" },
  "imagelayer.focusAndStars": { en: "Focus & stars", fr: "Mise au point et étoiles", de: "Fokus & Sterne", it: "Messa a fuoco e stelle", es: "Enfoque y estrellas" },
  "imagelayer.statistics": { en: "Statistics", fr: "Statistiques", de: "Statistik", it: "Statistiche", es: "Estadísticas" },
  "imagelayer.astrometry": { en: "Astrometry", fr: "Astrométrie", de: "Astrometrie", it: "Astrometria", es: "Astrometría" },
  "imagelayer.imageStats": { en: "Image stats", fr: "Statistiques de l'image", de: "Bildstatistik", it: "Statistiche immagine", es: "Estadísticas de la imagen" },
  "imagelayer.stats": { en: "Stats", fr: "Stats", de: "Statistik", it: "Statistiche", es: "Estadísticas" },
  "imagelayer.channelR": { en: "R", fr: "R", de: "R", it: "R", es: "R" },
  "imagelayer.channelG": { en: "G", fr: "V", de: "G", it: "V", es: "V" },
  "imagelayer.channelB": { en: "B", fr: "B", de: "B", it: "B", es: "A" },
  "imagelayer.channelValue": { en: "Value", fr: "Valeur", de: "Wert", it: "Valore", es: "Valor" },

  // ---- views.js: modbar system entries, login, controller/files ----------
  "views.controller": { en: "Controller", fr: "Contrôleur", de: "Steuerung", it: "Controllore", es: "Controlador" },
  "views.files": { en: "Files", fr: "Fichiers", de: "Dateien", it: "File", es: "Archivos" },
  "views.logs": { en: "Logs", fr: "Journaux", de: "Protokolle", it: "Log", es: "Registros" },
  "views.loginRequired": { en: "Login required", fr: "Connexion requise", de: "Anmeldung erforderlich", it: "Accesso richiesto", es: "Inicio de sesión requerido" },
  "views.loginRequiresAuth": { en: "This server requires authentication (grant-client = -1).", fr: "Ce serveur nécessite une authentification (grant-client = -1).", de: "Dieser Server erfordert eine Authentifizierung (grant-client = -1).", it: "Questo server richiede l'autenticazione (grant-client = -1).", es: "Este servidor requiere autenticación (grant-client = -1)." },
  "views.user": { en: "User", fr: "Utilisateur", de: "Benutzer", it: "Utente", es: "Usuario" },
  "views.password": { en: "Password", fr: "Mot de passe", de: "Passwort", it: "Password", es: "Contraseña" },
  "views.userPlaceholder": { en: "user", fr: "utilisateur", de: "Benutzer", it: "utente", es: "usuario" },
  "views.passwordPlaceholder": { en: "password", fr: "mot de passe", de: "Passwort", it: "password", es: "contraseña" },
  "views.serverLoad": { en: "Server load", fr: "Charge serveur", de: "Serverlast", it: "Carico server", es: "Carga del servidor" },
  "views.cpuLabel": { en: "CPU (1/5/15m)", fr: "CPU (1/5/15 min)", de: "CPU (1/5/15 Min)", it: "CPU (1/5/15 min)", es: "CPU (1/5/15 min)" },
  "views.ramLabel": { en: "RAM", fr: "RAM", de: "RAM", it: "RAM", es: "RAM" },
  "views.configurations": { en: "Configurations", fr: "Configurations", de: "Konfigurationen", it: "Configurazioni", es: "Configuraciones" },
  "views.loadedModules": { en: "Loaded modules", fr: "Modules chargés", de: "Geladene Module", it: "Moduli caricati", es: "Módulos cargados" },
  "views.loadAModule": { en: "Load a module", fr: "Charger un module", de: "Modul laden", it: "Carica un modulo", es: "Cargar un módulo" },
  "views.configNamePlaceholder": { en: "configuration name", fr: "nom de la configuration", de: "Konfigurationsname", it: "nome configurazione", es: "nombre de la configuración" },
  "views.instanceLabelPlaceholder": { en: "instance label", fr: "libellé de l'instance", de: "Instanzbezeichnung", it: "etichetta istanza", es: "etiqueta de instancia" },
  "views.profileOptionalPlaceholder": { en: "profile (optional)", fr: "profil (optionnel)", de: "Profil (optional)", it: "profilo (opzionale)", es: "perfil (opcional)" },
  "views.noFilesInFolder": { en: "No files in this folder.", fr: "Aucun fichier dans ce dossier.", de: "Keine Dateien in diesem Ordner.", it: "Nessun file in questa cartella.", es: "No hay archivos en esta carpeta." },
  "views.login": { en: "Login", fr: "Connexion", de: "Anmelden", it: "Accedi", es: "Iniciar sesión" },
  "views.saveCurrentAs": { en: "Save current as…", fr: "Enregistrer sous…", de: "Aktuelle speichern als…", it: "Salva attuale come…", es: "Guardar actual como…" },
  "views.load": { en: "Load", fr: "Charger", de: "Laden", it: "Carica", es: "Cargar" },
  "views.kill": { en: "Kill", fr: "Arrêter", de: "Beenden", it: "Termina", es: "Detener" },
  "views.unloadConfirm": { en: "Unload {name}?", fr: "Décharger {name} ?", de: "{name} entladen?", it: "Scaricare {name}?", es: "¿Descargar {name}?" },
  "views.indiServerPrefix": { en: "INDI server — ", fr: "Serveur INDI — ", de: "INDI-Server — ", it: "Server INDI — ", es: "Servidor INDI — " },
  "views.enabled": { en: "enabled", fr: "activé", de: "aktiviert", it: "attivo", es: "activado" },
  "views.disabled": { en: "disabled", fr: "désactivé", de: "deaktiviert", it: "disattivato", es: "desactivado" },
  "views.startServer": { en: "Start server", fr: "Démarrer le serveur", de: "Server starten", it: "Avvia server", es: "Iniciar servidor" },
  "views.stopServer": { en: "Stop server", fr: "Arrêter le serveur", de: "Server stoppen", it: "Ferma server", es: "Detener servidor" },
  "views.reload": { en: "Reload", fr: "Recharger", de: "Neu laden", it: "Ricarica", es: "Recargar" },
  "views.stop": { en: "Stop", fr: "Arrêter", de: "Stoppen", it: "Ferma", es: "Detener" },
  "views.runningSuffix": { en: " ● running", fr: " ● en cours", de: " ● läuft", it: " ● in esecuzione", es: " ● en ejecución" },
  "views.rootFolder": { en: "(root)", fr: "(racine)", de: "(Wurzel)", it: "(radice)", es: "(raíz)" },
  "views.selectedPrefix": { en: "Selected: ", fr: "Sélectionné : ", de: "Ausgewählt: ", it: "Selezionato: ", es: "Seleccionado: " },
  "views.none": { en: "(none)", fr: "(aucun)", de: "(keine)", it: "(nessuno)", es: "(ninguno)" },
  "views.entriesCappedSuffix": { en: "entries (capped at 1000)", fr: "entrées (limité à 1000)", de: "Einträge (auf 1000 begrenzt)", it: "voci (limitato a 1000)", es: "entradas (limitado a 1000)" },
  "views.current": { en: " (current)", fr: " (actuelle)", de: " (aktuell)", it: " (attuale)", es: " (actual)" },
  "views.moduleCountSuffix": { en: "module(s)", fr: "module(s)", de: "Modul(e)", it: "modulo/i", es: "módulo(s)" },

  // ---- templates/navigator.js: the planetarium screen ---------------------
  "navigator.tapToSelect": { en: "Tap a star or object on the sky to select it.", fr: "Touchez une étoile ou un objet du ciel pour le sélectionner.", de: "Tippen Sie auf einen Stern oder ein Objekt am Himmel, um es auszuwählen.", it: "Tocca una stella o un oggetto del cielo per selezionarlo.", es: "Toque una estrella u objeto del cielo para seleccionarlo." },
  "navigator.goto": { en: "GOTO", fr: "GOTO", de: "GOTO", it: "GOTO", es: "GOTO" },
  "navigator.addToPlanner": { en: "Add to planner", fr: "Ajouter au planificateur", de: "Zum Planer hinzufügen", it: "Aggiungi al pianificatore", es: "Añadir al planificador" },
  "navigator.star": { en: "Star", fr: "Étoile", de: "Stern", it: "Stella", es: "Estrella" },
  "navigator.dso": { en: "DSO", fr: "OCP", de: "DSO", it: "OCP", es: "OCP" },
  "navigator.point": { en: "Point", fr: "Point", de: "Punkt", it: "Punto", es: "Punto" },
  "navigator.magPrefix": { en: "mag ", fr: "mag ", de: "mag ", it: "mag ", es: "mag " },
  "navigator.constellations": { en: "Constellations", fr: "Constellations", de: "Sternbilder", it: "Costellazioni", es: "Constelaciones" },
  "navigator.cardinalPoints": { en: "Cardinal points", fr: "Points cardinaux", de: "Himmelsrichtungen", it: "Punti cardinali", es: "Puntos cardinales" },
  "navigator.messierObjects": { en: "Messier objects", fr: "Objets Messier", de: "Messier-Objekte", it: "Oggetti Messier", es: "Objetos Messier" },
  "navigator.trueHorizon": { en: "True horizon", fr: "Horizon réel", de: "Wahrer Horizont", it: "Orizzonte reale", es: "Horizonte real" },
  "navigator.equatorialGrid": { en: "Equatorial grid", fr: "Grille équatoriale", de: "Äquatorialgitter", it: "Griglia equatoriale", es: "Cuadrícula ecuatorial" },
  "navigator.azimuthalGrid": { en: "Azimuthal grid", fr: "Grille azimutale", de: "Azimutalgitter", it: "Griglia azimutale", es: "Cuadrícula azimutal" },
  "navigator.meridian": { en: "Meridian", fr: "Méridien", de: "Meridian", it: "Meridiano", es: "Meridiano" },
  "navigator.dayMode": { en: "Day mode", fr: "Mode jour", de: "Tagmodus", it: "Modalità giorno", es: "Modo día" },
  "navigator.stereographic": { en: "Stereographic projection", fr: "Projection stéréographique", de: "Stereografische Projektion", it: "Proiezione stereografica", es: "Proyección estereográfica" },
  "navigator.catalogDepth": { en: "Star catalog", fr: "Catalogue d'étoiles", de: "Sternkatalog", it: "Catalogo stellare", es: "Catálogo de estrellas" },
  "navigator.catalogDepth.light": { en: "Light (mag ≤ 6.5, ~8.9k stars)", fr: "Léger (mag ≤ 6,5, ~8,9k étoiles)", de: "Leicht (mag ≤ 6,5, ~8.900 Sterne)", it: "Leggero (mag ≤ 6,5, ~8,9k stelle)", es: "Ligero (mag ≤ 6,5, ~8,9k estrellas)" },
  "navigator.catalogDepth.deep": { en: "Deep (mag ≤ 8, ~41k stars)", fr: "Profond (mag ≤ 8, ~41k étoiles)", de: "Tief (mag ≤ 8, ~41.000 Sterne)", it: "Profondo (mag ≤ 8, ~41k stelle)", es: "Profundo (mag ≤ 8, ~41k estrellas)" },
  "navigator.catalogDepth.full": { en: "Full (Hipparcos, ~118k stars)", fr: "Complet (Hipparcos, ~118k étoiles)", de: "Vollständig (Hipparcos, ~118.000 Sterne)", it: "Completo (Hipparcos, ~118k stelle)", es: "Completo (Hipparcos, ~118k estrellas)" },
  "navigator.zoom": { en: "Zoom", fr: "Zoom", de: "Zoom", it: "Zoom", es: "Zoom" },
  "navigator.selection": { en: "Selection", fr: "Sélection", de: "Auswahl", it: "Selezione", es: "Selección" },
  "navigator.displaySettings": { en: "Display settings", fr: "Réglages d'affichage", de: "Anzeigeeinstellungen", it: "Impostazioni di visualizzazione", es: "Ajustes de visualización" },
  "navigator.settingsQuicknav": { en: "Settings", fr: "Réglages", de: "Einstellungen", it: "Impostazioni", es: "Ajustes" },

  // ---- templates/focus.js --------------------------------------------------
  "focus.focusCurve": { en: "Focus curve", fr: "Courbe de mise au point", de: "Fokuskurve", it: "Curva di messa a fuoco", es: "Curva de enfoque" },

  // ---- templates/guider.js -------------------------------------------------
  "guider.sessionValues": { en: "Session values", fr: "Valeurs de la session", de: "Sitzungswerte", it: "Valori della sessione", es: "Valores de la sesión" },
  "guider.guiding": { en: "Guiding", fr: "Guidage", de: "Nachführung", it: "Guida", es: "Guiado" },
  "guider.drift": { en: "Drift", fr: "Dérive", de: "Drift", it: "Deriva", es: "Deriva" },
  "guider.pulse": { en: "Pulse", fr: "Impulsion", de: "Impuls", it: "Impulso", es: "Impulso" },
  "guider.snr": { en: "SNR", fr: "RSB", de: "SNR", it: "SNR", es: "SNR" },

  // ---- sky-engine.js: Sun/Moon/planet labels drawn on the sky dome --------
  "sky.sun": { en: "Sun", fr: "Soleil", de: "Sonne", it: "Sole", es: "Sol" },
  "sky.moon": { en: "Moon", fr: "Lune", de: "Mond", it: "Luna", es: "Luna" },
  "sky.mercury": { en: "Mercury", fr: "Mercure", de: "Merkur", it: "Mercurio", es: "Mercurio" },
  "sky.venus": { en: "Venus", fr: "Vénus", de: "Venus", it: "Venere", es: "Venus" },
  "sky.mars": { en: "Mars", fr: "Mars", de: "Mars", it: "Marte", es: "Marte" },
  "sky.jupiter": { en: "Jupiter", fr: "Jupiter", de: "Jupiter", it: "Giove", es: "Júpiter" },
  "sky.saturn": { en: "Saturn", fr: "Saturne", de: "Saturn", it: "Saturno", es: "Saturno" },
};

/** Looks up `key` in the current language, falling back to English, then
 * to the bare key itself (visibly wrong, but never throws — a missing
 * translation should degrade, not crash a panel). `vars`, if given,
 * substitutes `{token}` placeholders (e.g. t("views.unloadConfirm",
 * {name: "guider1"})) - the one bit of interpolation this deliberately
 * small module supports, needed for the handful of local sentences built
 * around a dynamic value (a module name, a count...). */
export function t(key, vars) {
  const entry = STRINGS[key];
  let str;
  if (!entry) {
    console.warn(`[i18n] missing key "${key}"`);
    str = key;
  } else {
    str = entry[currentLang] || entry[DEFAULT_LANG] || key;
  }
  if (vars) str = str.replace(/\{(\w+)\}/g, (m, k) => (k in vars ? vars[k] : m));
  return str;
}
