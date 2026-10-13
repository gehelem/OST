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
  "common.actions": { en: "Actions", fr: "Actions", de: "Aktionen", it: "Azioni", es: "Acciones" },
  "common.moduleInfo": { en: "Info", fr: "Infos", de: "Info", it: "Info", es: "Información" },
  "common.disableTemplates": { en: "Disable templates", fr: "Désactiver les templates", de: "Vorlagen deaktivieren", it: "Disattiva i template", es: "Desactivar las plantillas" },
  "common.disableTemplatesHint": { en: "Force the generic property view for this module (dev mode)", fr: "Force la vue générique des propriétés pour ce module (mode développement)", de: "Erzwingt die generische Eigenschaftsansicht für dieses Modul (Entwicklermodus)", it: "Forza la vista generica delle proprietà per questo modulo (modalità sviluppo)", es: "Fuerza la vista genérica de propiedades para este módulo (modo desarrollo)" },
  "common.moduleNotFound": { en: "Module not found (unloaded?)", fr: "Module introuvable (déchargé ?)", de: "Modul nicht gefunden (entladen?)", it: "Modulo non trovato (scaricato?)", es: "Módulo no encontrado (¿descargado?)" },
  "common.general": { en: "General", fr: "Général", de: "Allgemein", it: "Generale", es: "General" },
  "common.close": { en: "Close", fr: "Fermer", de: "Schließen", it: "Chiudi", es: "Cerrar" },
  "common.cancel": { en: "Cancel", fr: "Annuler", de: "Abbrechen", it: "Annulla", es: "Cancelar" },
  "common.save": { en: "Save", fr: "Enregistrer", de: "Speichern", it: "Salva", es: "Guardar" },
  "common.editProperty": { en: "Edit", fr: "Modifier", de: "Bearbeiten", it: "Modifica", es: "Editar" },
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
  "chrome.parameters": { en: "Parameters", fr: "Paramètres", de: "Parameter", it: "Parametri", es: "Parámetros" },
  "chrome.uiSize": { en: "UI size", fr: "Taille de l'interface", de: "Oberflächengröße", it: "Dimensione interfaccia", es: "Tamaño de la interfaz" },
  "chrome.toggleTheme": { en: "Toggle theme", fr: "Changer de thème", de: "Design umschalten", it: "Cambia tema", es: "Cambiar tema" },
  "chrome.login": { en: "Login", fr: "Connexion", de: "Anmelden", it: "Accedi", es: "Iniciar sesión" },
  "chrome.logs": { en: "Logs", fr: "Journaux", de: "Protokolle", it: "Log", es: "Registros" },
  "chrome.connect": { en: "Connect", fr: "Connecter", de: "Verbinden", it: "Connetti", es: "Conectar" },
  "chrome.disconnect": { en: "Disconnect", fr: "Déconnecter", de: "Trennen", it: "Disconnetti", es: "Desconectar" },
  "chrome.resize": { en: "Resize", fr: "Redimensionner", de: "Größe ändern", it: "Ridimensiona", es: "Redimensionar" },
  "chrome.parametersPrefix": { en: "Parameters — ", fr: "Paramètres — ", de: "Parameter — ", it: "Parametri — ", es: "Parámetros — " },
  "chrome.notConnectedLede": { en: "Not connected. Set the WebSocket URL above and press Connect.", fr: "Non connecté. Renseignez l'URL WebSocket ci-dessus et cliquez sur Connecter.", de: "Nicht verbunden. Geben Sie oben die WebSocket-URL ein und klicken Sie auf Verbinden.", it: "Non connesso. Imposta l'URL WebSocket sopra e premi Connetti.", es: "No conectado. Indique la URL WebSocket arriba y pulse Conectar." },
  "chrome.recentServers": { en: "Recent servers", fr: "Serveurs récents", de: "Letzte Server", it: "Server recenti", es: "Servidores recientes" },
  "chrome.noHistoryYet": { en: "No history yet.", fr: "Aucun historique pour l'instant.", de: "Noch kein Verlauf.", it: "Ancora nessuna cronologia.", es: "Aún no hay historial." },
  "chrome.removeServer": { en: "Remove", fr: "Supprimer", de: "Entfernen", it: "Rimuovi", es: "Eliminar" },
  "chrome.clearCache": { en: "Clear cache + localStorage", fr: "Vider le cache + localStorage", de: "Cache + localStorage leeren", it: "Svuota cache + localStorage", es: "Vaciar caché + localStorage" },
  "chrome.clearCacheConfirm": { en: "Clear all locally stored settings/layout and reload? You'll need to reconnect.", fr: "Vider tous les réglages/dispositions stockés localement et recharger ? Il faudra se reconnecter.", de: "Alle lokal gespeicherten Einstellungen/Layouts löschen und neu laden? Eine erneute Verbindung ist danach nötig.", it: "Svuotare tutte le impostazioni/layout salvati localmente e ricaricare? Dovrai riconnetterti.", es: "¿Vaciar todos los ajustes/diseños guardados localmente y recargar? Tendrás que reconectar." },
  "chrome.discover": { en: "Discover", fr: "Découvrir", de: "Entdecken", it: "Scopri", es: "Descubrir" },
  "chrome.discovering": { en: "Scanning the local network…", fr: "Recherche sur le réseau local…", de: "Lokales Netzwerk wird durchsucht…", it: "Scansione della rete locale…", es: "Buscando en la red local…" },
  "chrome.discoverNone": { en: "No server found.", fr: "Aucun serveur trouvé.", de: "Kein Server gefunden.", it: "Nessun server trovato.", es: "No se encontró ningún servidor." },

  // ---- imagelayer.js: shared image/stats block ----------------------------
  "imagelayer.dimensions": { en: "Dimensions", fr: "Dimensions", de: "Abmessungen", it: "Dimensioni", es: "Dimensiones" },
  "imagelayer.focus": { en: "Focus", fr: "Mise au point", de: "Fokus", it: "Messa a fuoco", es: "Enfoque" },
  "imagelayer.statistics": { en: "Statistics", fr: "Statistiques", de: "Statistik", it: "Statistiche", es: "Estadísticas" },
  "imagelayer.astrometry": { en: "Astrometry", fr: "Astrométrie", de: "Astrometrie", it: "Astrometria", es: "Astrometría" },
  "imagelayer.imageStats": { en: "Image stats", fr: "Statistiques de l'image", de: "Bildstatistik", it: "Statistiche immagine", es: "Estadísticas de la imagen" },
  "imagelayer.stats": { en: "Stats", fr: "Stats", de: "Statistik", it: "Statistiche", es: "Estadísticas" },
  "imagelayer.histogram": { en: "Histogram", fr: "Histogramme", de: "Histogramm", it: "Istogramma", es: "Histograma" },
  "imagelayer.channelR": { en: "R", fr: "R", de: "R", it: "R", es: "R" },
  "imagelayer.channelG": { en: "G", fr: "V", de: "G", it: "V", es: "V" },
  "imagelayer.channelB": { en: "B", fr: "B", de: "B", it: "B", es: "A" },
  "imagelayer.channelValue": { en: "Value", fr: "Valeur", de: "Wert", it: "Valore", es: "Valor" },
  "imagelayer.noAlternates": { en: "No image yet", fr: "Aucune image pour l'instant", de: "Noch kein Bild", it: "Nessuna immagine disponibile", es: "Aún no hay imagen" },

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
  "views.killAll": { en: "Kill all", fr: "Tout arrêter", de: "Alle beenden", it: "Termina tutti", es: "Detener todos" },
  "views.killAllConfirm": { en: "Unload all {count} loaded modules?", fr: "Décharger les {count} modules chargés ?", de: "Alle {count} geladenen Module entladen?", it: "Scaricare tutti i {count} moduli caricati?", es: "¿Descargar los {count} módulos cargados?" },
  "views.indiServerPrefix": { en: "INDI server — ", fr: "Serveur INDI — ", de: "INDI-Server — ", it: "Server INDI — ", es: "Servidor INDI — " },
  "views.enabled": { en: "enabled", fr: "activé", de: "aktiviert", it: "attivo", es: "activado" },
  "views.disabled": { en: "disabled", fr: "désactivé", de: "deaktiviert", it: "disattivato", es: "desactivado" },
  "views.indiServerManagedElsewhere": { en: "The INDI server is managed outside OST — nothing to do here.", fr: "Le serveur INDI est géré en dehors d'OST — rien à faire ici.", de: "Der INDI-Server wird außerhalb von OST verwaltet — hier ist nichts zu tun.", it: "Il server INDI è gestito fuori da OST — niente da fare qui.", es: "El servidor INDI se gestiona fuera de OST — no hay nada que hacer aquí." },
  "views.startServer": { en: "Start server", fr: "Démarrer le serveur", de: "Server starten", it: "Avvia server", es: "Iniciar servidor" },
  "views.stopServer": { en: "Stop server", fr: "Arrêter le serveur", de: "Server stoppen", it: "Ferma server", es: "Detener servidor" },
  "views.reload": { en: "Reload", fr: "Recharger", de: "Neu laden", it: "Ricarica", es: "Recargar" },
  "views.stop": { en: "Stop", fr: "Arrêter", de: "Stoppen", it: "Ferma", es: "Detener" },
  "views.connect": { en: "Connect", fr: "Connecter", de: "Verbinden", it: "Connetti", es: "Conectar" },
  "views.connectAll": { en: "Connect all", fr: "Tout connecter", de: "Alle verbinden", it: "Connetti tutti", es: "Conectar todos" },
  "views.disconnect": { en: "Disconnect", fr: "Déconnecter", de: "Trennen", it: "Disconnetti", es: "Desconectar" },
  "views.runningSuffix": { en: " ● running", fr: " ● en cours", de: " ● läuft", it: " ● in esecuzione", es: " ● en ejecución" },
  "views.activeIndiDrivers": { en: "Active drivers", fr: "Drivers actifs", de: "Aktive Treiber", it: "Driver attivi", es: "Controladores activos" },
  "views.start": { en: "Start", fr: "Démarrer", de: "Starten", it: "Avvia", es: "Iniciar" },
  "views.indiDriverSearchPlaceholder": { en: "Search a driver… (3+ characters)", fr: "Rechercher un driver… (3 caractères min.)", de: "Treiber suchen… (mind. 3 Zeichen)", it: "Cerca un driver… (3+ caratteri)", es: "Buscar un controlador… (3+ caracteres)" },
  "views.startIndiDriver": { en: "Start a driver", fr: "Démarrer un driver", de: "Treiber starten", it: "Avvia un driver", es: "Iniciar un controlador" },
  "views.rootFolder": { en: "(root)", fr: "(racine)", de: "(Wurzel)", it: "(radice)", es: "(raíz)" },
  "views.selectedPrefix": { en: "Selected: ", fr: "Sélectionné : ", de: "Ausgewählt: ", it: "Selezionato: ", es: "Seleccionado: " },
  "views.none": { en: "(none)", fr: "(aucun)", de: "(keine)", it: "(nessuno)", es: "(ninguno)" },
  "views.entriesCappedSuffix": { en: "entries (capped at 1000)", fr: "entrées (limité à 1000)", de: "Einträge (auf 1000 begrenzt)", it: "voci (limitato a 1000)", es: "entradas (limitado a 1000)" },
  "views.logModuleAll": { en: "All modules", fr: "Tous les modules", de: "Alle Module", it: "Tutti i moduli", es: "Todos los módulos" },
  "views.logLevelAll": { en: "All levels", fr: "Tous les niveaux", de: "Alle Stufen", it: "Tutti i livelli", es: "Todos los niveles" },
  "views.logLevelInfo": { en: "Info and above", fr: "Info et plus", de: "Info und höher", it: "Info e superiori", es: "Info y superior" },
  "views.logLevelWarning": { en: "Warning and above", fr: "Avertissement et plus", de: "Warnung und höher", it: "Avviso e superiori", es: "Aviso y superior" },
  "views.logLevelError": { en: "Error and above", fr: "Erreur et plus", de: "Fehler und höher", it: "Errore e superiori", es: "Error y superior" },
  "views.logLevelCritical": { en: "Critical only", fr: "Critique seulement", de: "Nur kritisch", it: "Solo critico", es: "Solo crítico" },
  "views.current": { en: " (current)", fr: " (actuelle)", de: " (aktuell)", it: " (attuale)", es: " (actual)" },
  "views.moduleCountSuffix": { en: "module(s)", fr: "module(s)", de: "Modul(e)", it: "modulo/i", es: "módulo(s)" },

  // ---- views.js: home dashboard (one read-only card per loaded module) ---
  "views.home": { en: "Home", fr: "Accueil", de: "Start", it: "Home", es: "Inicio" },
  "views.noModuleLoaded": { en: "No module loaded.", fr: "Aucun module chargé.", de: "Kein Modul geladen.", it: "Nessun modulo caricato.", es: "No hay ningún módulo cargado." },
  "views.homeGeneric": { en: "Click to open", fr: "Cliquer pour ouvrir", de: "Zum Öffnen klicken", it: "Clicca per aprire", es: "Clic para abrir" },
  "views.homeHfr": { en: "HFR", fr: "HFR", de: "HFR", it: "HFR", es: "HFR" },
  "views.homePosition": { en: "Position", fr: "Position", de: "Position", it: "Posizione", es: "Posición" },
  "views.homeProgress": { en: "Progress", fr: "Progression", de: "Fortschritt", it: "Avanzamento", es: "Progreso" },
  "views.homeRemaining": { en: "Remaining", fr: "Restant", de: "Verbleibend", it: "Rimanente", es: "Restante" },
  "views.homeExposure": { en: "Exposure", fr: "Exposition", de: "Belichtung", it: "Esposizione", es: "Exposición" },
  "views.homeTarget": { en: "Target", fr: "Cible", de: "Ziel", it: "Obiettivo", es: "Objetivo" },
  "views.homeDragHint": { en: "Drag to reorder", fr: "Glisser pour réorganiser", de: "Ziehen zum Anordnen", it: "Trascina per riordinare", es: "Arrastrar para reordenar" },

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
  "navigator.planets": { en: "Planets", fr: "Planètes", de: "Planeten", it: "Pianeti", es: "Planetas" },
  "navigator.trueHorizon": { en: "True horizon", fr: "Horizon réel", de: "Wahrer Horizont", it: "Orizzonte reale", es: "Horizonte real" },
  "navigator.equatorialGrid": { en: "Equatorial grid", fr: "Grille équatoriale", de: "Äquatorialgitter", it: "Griglia equatoriale", es: "Cuadrícula ecuatorial" },
  "navigator.azimuthalGrid": { en: "Azimuthal grid", fr: "Grille azimutale", de: "Azimutalgitter", it: "Griglia azimutale", es: "Cuadrícula azimutal" },
  "navigator.meridian": { en: "Meridian", fr: "Méridien", de: "Meridian", it: "Meridiano", es: "Meridiano" },
  "navigator.mountPosition": { en: "Telescope position", fr: "Position du télescope", de: "Teleskopposition", it: "Posizione del telescopio", es: "Posición del telescopio" },
  "navigator.sensorFov": { en: "Sensor field of view", fr: "Champ du capteur", de: "Sensor-Gesichtsfeld", it: "Campo del sensore", es: "Campo del sensor" },
  "navigator.milkyWay": { en: "Milky Way", fr: "Voie lactée", de: "Milchstraße", it: "Via Lattea", es: "Vía Láctea" },
  "navigator.ecliptic": { en: "Ecliptic", fr: "Écliptique", de: "Ekliptik", it: "Eclittica", es: "Eclíptica" },
  "navigator.objectNames": { en: "Object names", fr: "Noms des objets", de: "Objektnamen", it: "Nomi degli oggetti", es: "Nombres de objetos" },
  "navigator.sensorImage": { en: "Captured image in frame", fr: "Image capturée dans le cadre", de: "Aufgenommenes Bild im Rahmen", it: "Immagine catturata nel riquadro", es: "Imagen capturada en el marco" },
  "navigator.dayMode": { en: "Day mode", fr: "Mode jour", de: "Tagmodus", it: "Modalità giorno", es: "Modo día" },
  "navigator.stereographic": { en: "Stereographic projection", fr: "Projection stéréographique", de: "Stereografische Projektion", it: "Proiezione stereografica", es: "Proyección estereográfica" },
  "navigator.catalogDepth": { en: "Star catalog", fr: "Catalogue d'étoiles", de: "Sternkatalog", it: "Catalogo stellare", es: "Catálogo de estrellas" },
  "navigator.catalogDepth.light": { en: "Light (mag ≤ 6.5, ~8.9k stars)", fr: "Léger (mag ≤ 6,5, ~8,9k étoiles)", de: "Leicht (mag ≤ 6,5, ~8.900 Sterne)", it: "Leggero (mag ≤ 6,5, ~8,9k stelle)", es: "Ligero (mag ≤ 6,5, ~8,9k estrellas)" },
  "navigator.catalogDepth.deep": { en: "Deep (mag ≤ 8, ~41k stars)", fr: "Profond (mag ≤ 8, ~41k étoiles)", de: "Tief (mag ≤ 8, ~41.000 Sterne)", it: "Profondo (mag ≤ 8, ~41k stelle)", es: "Profundo (mag ≤ 8, ~41k estrellas)" },
  "navigator.catalogDepth.full": { en: "Full (Hipparcos, ~118k stars)", fr: "Complet (Hipparcos, ~118k étoiles)", de: "Vollständig (Hipparcos, ~118.000 Sterne)", it: "Completo (Hipparcos, ~118k stelle)", es: "Completo (Hipparcos, ~118k estrellas)" },
  "navigator.catalogDepth.lightShort": { en: "Light", fr: "Léger", de: "Leicht", it: "Leggero", es: "Ligero" },
  "navigator.catalogDepth.deepShort": { en: "Deep", fr: "Profond", de: "Tief", it: "Profondo", es: "Profundo" },
  "navigator.catalogDepth.fullShort": { en: "Full", fr: "Complet", de: "Vollständig", it: "Completo", es: "Completo" },
  "navigator.zoom": { en: "Zoom", fr: "Zoom", de: "Zoom", it: "Zoom", es: "Zoom" },
  "navigator.selection": { en: "Selection", fr: "Sélection", de: "Auswahl", it: "Selezione", es: "Selección" },
  "navigator.displaySettings": { en: "Display settings", fr: "Réglages d'affichage", de: "Anzeigeeinstellungen", it: "Impostazioni di visualizzazione", es: "Ajustes de visualización" },
  "navigator.settingsQuicknav": { en: "Settings", fr: "Réglages", de: "Einstellungen", it: "Impostazioni", es: "Ajustes" },
  "navigator.timeNav": { en: "Time navigator", fr: "Navigation temporelle", de: "Zeitnavigator", it: "Navigatore temporale", es: "Navegador temporal" },
  "navigator.timeNavQuicknav": { en: "Time", fr: "Temps", de: "Zeit", it: "Tempo", es: "Tiempo" },
  "navigator.centerOnMount": { en: "Center on mount", fr: "Centrer sur la monture", de: "Auf Montierung zentrieren", it: "Centra sulla montatura", es: "Centrar en la montura" },
  "navigator.search": { en: "Search", fr: "Recherche", de: "Suche", it: "Ricerca", es: "Búsqueda" },
  "navigator.searchPlaceholder": { en: "Object name...", fr: "Nom de l'objet...", de: "Objektname...", it: "Nome dell'oggetto...", es: "Nombre del objeto..." },
  "navigator.searchLaunch": { en: "Search", fr: "Rechercher", de: "Suchen", it: "Cerca", es: "Buscar" },
  "navigator.searchCatalogsBtn": { en: "Catalogs to search", fr: "Catalogues à interroger", de: "Zu durchsuchende Kataloge", it: "Cataloghi da cercare", es: "Catálogos a buscar" },
  "navigator.searchNoResults": { en: "No results", fr: "Aucun résultat", de: "Keine Ergebnisse", it: "Nessun risultato", es: "Sin resultados" },
  "navigator.searchColName": { en: "Name", fr: "Nom", de: "Name", it: "Nome", es: "Nombre" },
  "navigator.searchColCatalog": { en: "Catalog", fr: "Catalogue", de: "Katalog", it: "Catalogo", es: "Catálogo" },
  "navigator.searchColCoords": { en: "Coordinates", fr: "Coordonnées", de: "Koordinaten", it: "Coordinate", es: "Coordenadas" },
  "navigator.searchColMag": { en: "Mag", fr: "Mag", de: "Mag", it: "Mag", es: "Mag" },
  "navigator.searchCatalogsTitle": { en: "Search catalogs", fr: "Catalogues de recherche", de: "Suchkataloge", it: "Cataloghi di ricerca", es: "Catálogos de búsqueda" },
  "navigator.searchLocalCatalogsLabel": { en: "Local (built into the app)", fr: "Locaux (intégrés à l'application)", de: "Lokal (in der App enthalten)", it: "Locali (integrati nell'app)", es: "Locales (integrados en la app)" },
  "navigator.searchBackendCatalogsLabel": { en: "Hosted on the server", fr: "Hébergés sur le serveur", de: "Auf dem Server gehostet", it: "Ospitati sul server", es: "Alojados en el servidor" },
  "navigator.searchCatalog.stars": { en: "Stars", fr: "Étoiles", de: "Sterne", it: "Stelle", es: "Estrellas" },
  "navigator.searchCatalog.messier": { en: "Messier", fr: "Messier", de: "Messier", it: "Messier", es: "Messier" },
  "navigator.searchCatalog.planets": { en: "Planets", fr: "Planètes", de: "Planeten", it: "Pianeti", es: "Planetas" },
  "navigator.searchCatalog.backend": { en: "Backend", fr: "Backend", de: "Backend", it: "Backend", es: "Backend" },
  "navigator.nowBtn": { en: "Now", fr: "Maintenant", de: "Jetzt", it: "Adesso", es: "Ahora" },
  "navigator.rise": { en: "Rise", fr: "Lever", de: "Aufgang", it: "Levata", es: "Salida" },
  "navigator.transit": { en: "Meridian passage", fr: "Passage méridien", de: "Meridiandurchgang", it: "Passaggio al meridiano", es: "Paso meridiano" },
  "navigator.set": { en: "Set", fr: "Coucher", de: "Untergang", it: "Tramonto", es: "Puesta" },
  "navigator.maxAltitude": { en: "Max altitude", fr: "Hauteur maximale", de: "Max. Höhe", it: "Altezza massima", es: "Altura máxima" },
  "navigator.airmassAtTransit": { en: "Airmass at transit", fr: "Masse d'air au passage", de: "Luftmasse beim Durchgang", it: "Massa d'aria al passaggio", es: "Masa de aire en el paso" },

  // ---- templates/focus.js --------------------------------------------------
  "focus.focusCurve": { en: "Focus curve", fr: "Courbe de mise au point", de: "Fokuskurve", it: "Curva di messa a fuoco", es: "Curva de enfoque" },
  "focus.tilt": { en: "Tilt", fr: "Tilt", de: "Tilt", it: "Tilt", es: "Tilt" },

  // ---- templates/inspector.js -----------------------------------------------
  "inspector.maps": { en: "Analysis maps", fr: "Cartes d'analyse", de: "Analysekarten", it: "Mappe di analisi", es: "Mapas de análisis" },

  // ---- templates/guider.js -------------------------------------------------
  "guider.sessionValues": { en: "Session values", fr: "Valeurs de la session", de: "Sitzungswerte", it: "Valori della sessione", es: "Valores de la sesión" },
  "guider.guiding": { en: "Guiding", fr: "Guidage", de: "Nachführung", it: "Guida", es: "Guiado" },
  "guider.drift": { en: "Drift", fr: "Dérive", de: "Drift", it: "Deriva", es: "Deriva" },
  "guider.pulse": { en: "Pulse", fr: "Impulsion", de: "Impuls", it: "Impulso", es: "Impulso" },
  "guider.snr": { en: "SNR", fr: "RSB", de: "SNR", it: "SNR", es: "SNR" },

  // ---- templates/monitor.js --------------------------------------------------
  "monitor.graph": { en: "Session graph", fr: "Graphique de session", de: "Sitzungsdiagramm", it: "Grafico sessione", es: "Gráfico de sesión" },
  "monitor.status": { en: "Status", fr: "Statut", de: "Status", it: "Stato", es: "Estado" },
  "monitor.rms": { en: "Guide RMS", fr: "RMS guidage", de: "Guiding-RMS", it: "RMS guida", es: "RMS de guiado" },
  "monitor.snr": { en: "SNR", fr: "RSB", de: "SNR", it: "SNR", es: "SNR" },
  "monitor.imageHfr": { en: "Image HFR", fr: "HFR image", de: "Bild-HFR", it: "HFR immagine", es: "HFR de imagen" },
  "monitor.focusHfr": { en: "Focus HFR", fr: "HFR focus", de: "Fokus-HFR", it: "HFR fuoco", es: "HFR de enfoque" },

  // ---- sky-engine.js: Sun/Moon/planet labels drawn on the sky dome --------
  "sky.sun": { en: "Sun", fr: "Soleil", de: "Sonne", it: "Sole", es: "Sol" },
  "sky.moon": { en: "Moon", fr: "Lune", de: "Mond", it: "Luna", es: "Luna" },
  "sky.mercury": { en: "Mercury", fr: "Mercure", de: "Merkur", it: "Mercurio", es: "Mercurio" },
  "sky.venus": { en: "Venus", fr: "Vénus", de: "Venus", it: "Venere", es: "Venus" },
  "sky.mars": { en: "Mars", fr: "Mars", de: "Mars", it: "Marte", es: "Marte" },
  "sky.jupiter": { en: "Jupiter", fr: "Jupiter", de: "Jupiter", it: "Giove", es: "Júpiter" },
  "sky.saturn": { en: "Saturn", fr: "Saturne", de: "Saturn", it: "Saturno", es: "Saturno" },

  // ---- wizard.js: setup wizard (issue #238) -------------------------------
  "wizard.title": { en: "Setup wizard", fr: "Assistant de configuration", de: "Einrichtungsassistent", it: "Assistente di configurazione", es: "Asistente de configuración" },
  "wizard.back": { en: "Back", fr: "Précédent", de: "Zurück", it: "Precedente", es: "Anterior" },
  "wizard.next": { en: "Next", fr: "Suivant", de: "Weiter", it: "Avanti", es: "Siguiente" },
  "wizard.finish": { en: "Finish", fr: "Terminer", de: "Fertigstellen", it: "Termina", es: "Finalizar" },
  "wizard.add": { en: "Add", fr: "Ajouter", de: "Hinzufügen", it: "Aggiungi", es: "Añadir" },
  "wizard.step.indi": { en: "INDI drivers", fr: "Pilotes INDI", de: "INDI-Treiber", it: "Driver INDI", es: "Controladores INDI" },
  "wizard.step.optics": { en: "Optics", fr: "Optiques", de: "Optik", it: "Ottiche", es: "Ópticas" },
  "wizard.step.equipment": { en: "Equipment", fr: "Équipement", de: "Ausrüstung", it: "Attrezzatura", es: "Equipo" },
  "wizard.step.modules": { en: "Modules", fr: "Modules", de: "Module", it: "Moduli", es: "Módulos" },
  "wizard.step.save": { en: "Save", fr: "Enregistrer", de: "Speichern", it: "Salva", es: "Guardar" },
  "wizard.indi.intro": { en: "Start the INDI drivers for your equipment. At least one active driver is needed to continue.", fr: "Démarrez les pilotes INDI de votre matériel. Au moins un pilote actif est nécessaire pour continuer.", de: "Starten Sie die INDI-Treiber für Ihre Ausrüstung. Mindestens ein aktiver Treiber ist erforderlich, um fortzufahren.", it: "Avvia i driver INDI per la tua attrezzatura. È necessario almeno un driver attivo per continuare.", es: "Inicie los controladores INDI de su equipo. Se necesita al menos un controlador activo para continuar." },
  "wizard.optics.intro": { en: "Pick an existing optical setup or create a new one (focal length, diameter). At least one is needed to continue.", fr: "Choisissez une configuration optique existante ou créez-en une nouvelle (focale, diamètre). Au moins une est nécessaire pour continuer.", de: "Wählen Sie eine vorhandene Optik oder erstellen Sie eine neue (Brennweite, Durchmesser). Mindestens eine ist erforderlich, um fortzufahren.", it: "Scegli una configurazione ottica esistente o creane una nuova (focale, diametro). È necessaria almeno una per continuare.", es: "Elija una configuración óptica existente o cree una nueva (focal, diámetro). Se necesita al menos una para continuar." },
  "wizard.optics.add": { en: "New optics", fr: "Nouvelle optique", de: "Neue Optik", it: "Nuova ottica", es: "Nueva óptica" },
  "wizard.equipment.intro": { en: "Assign a device to each role (camera, mount, focuser...) from the drivers started in the previous step. At least one equipment set is needed to continue.", fr: "Affectez un périphérique à chaque rôle (caméra, monture, focuser...) parmi les pilotes démarrés à l'étape précédente. Au moins un jeu d'équipement est nécessaire pour continuer.", de: "Weisen Sie jeder Rolle (Kamera, Montierung, Fokussierer...) ein Gerät aus den im vorherigen Schritt gestarteten Treibern zu. Mindestens ein Ausrüstungssatz ist erforderlich, um fortzufahren.", it: "Assegna un dispositivo a ogni ruolo (camera, montatura, focheggiatore...) tra i driver avviati nel passaggio precedente. È necessario almeno un set di attrezzatura per continuare.", es: "Asigne un dispositivo a cada función (cámara, montura, enfocador...) entre los controladores iniciados en el paso anterior. Se necesita al menos un conjunto de equipo para continuar." },
  "wizard.equipment.add": { en: "New equipment set", fr: "Nouveau jeu d'équipement", de: "Neuer Ausrüstungssatz", it: "Nuovo set di attrezzatura", es: "Nuevo conjunto de equipo" },
  "wizard.modules.intro": { en: "Pick a common prefix (e.g. \"S1\") and which modules to load under it (e.g. \"S1-Navigator\", \"S1-Planner\"...). At least one module with this prefix is needed to continue.", fr: "Choisissez un préfixe commun (ex. « S1 ») et les modules à charger sous ce préfixe (ex. « S1-Navigator », « S1-Planner »...). Au moins un module avec ce préfixe est nécessaire pour continuer.", de: "Wählen Sie ein gemeinsames Präfix (z. B. „S1“) und die darunter zu ladenden Module (z. B. „S1-Navigator“, „S1-Planner“...). Mindestens ein Modul mit diesem Präfix ist erforderlich, um fortzufahren.", it: "Scegli un prefisso comune (es. \"S1\") e quali moduli caricare con quel prefisso (es. \"S1-Navigator\", \"S1-Planner\"...). È necessario almeno un modulo con questo prefisso per continuare.", es: "Elija un prefijo común (p. ej. «S1») y qué módulos cargar con ese prefijo (p. ej. «S1-Navigator», «S1-Planner»...). Se necesita al menos un módulo con este prefijo para continuar." },
  "wizard.modules.prefixPlaceholder": { en: "prefix, e.g. S1", fr: "préfixe, ex. S1", de: "Präfix, z. B. S1", it: "prefisso, es. S1", es: "prefijo, p. ej. S1" },
  "wizard.modules.create": { en: "Load selected modules", fr: "Charger les modules sélectionnés", de: "Ausgewählte Module laden", it: "Carica i moduli selezionati", es: "Cargar los módulos seleccionados" },
  "wizard.modules.setSlaves": { en: "set slaves", fr: "attribuer les esclaves", de: "Slaves zuweisen", it: "assegna slave", es: "asignar esclavos" },
  "wizard.save.intro": { en: "Save this setup as a named configuration so it can be reloaded later.", fr: "Enregistrez cette configuration sous un nom pour pouvoir la recharger plus tard.", de: "Speichern Sie diese Einrichtung als benannte Konfiguration, um sie später erneut zu laden.", it: "Salva questa configurazione con un nome per poterla ricaricare in seguito.", es: "Guarde esta configuración con un nombre para poder volver a cargarla más tarde." },
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
