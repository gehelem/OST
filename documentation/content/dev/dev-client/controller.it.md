---
title: Controller, GlobalDatastore e file
weight: 30
description: "Ciò che non è un modulo ordinario: dati del controller, carico di sistema, configurazioni, modulo GlobalDatastore, file system"
---

Questa pagina descrive ciò che esce dallo schema « moduli → proprietà → elementi » delle pagine precedenti: i **dati del controller**, il modulo centrale **`GlobalDatastore`** e il **file system** del server. Un client che si limita a mostrare i moduli non ne ha bisogno; un client che vuole offrire ciò che propone una pagina di parametri globali (configurazioni, driver INDI, carico del server, attrezzatura, immagini) deve gestirli.

## I dati del controller (`controllerdata`)

Il controller è il componente del server che carica i moduli e pilota il server INDI integrato. Espone i propri dati nel dump, sotto la chiave `controllerdata` (vedi [`d`]({{% relref "server-messages.it.md" %}}#d)), poi li mantiene aggiornati con messaggi [`uc`]({{% relref "server-messages.it.md" %}}#uc).

| Chiave | Contenuto |
|---|---|
| `availableconfs` | Configurazioni di moduli salvate: oggetto `nome della configurazione → elenco di {modulelabel, moduletype, profilename}` |
| `currentconf` | Nome della configurazione corrente |
| `banner` | Banner del server (testo libero, opzione `--banner`) |
| `git` | Versione del server: `Gitdate`, `Githash`, `Gitmessage`, `Gittag` |
| `libraries` | Librerie di moduli disponibili, un oggetto di metadati per libreria |
| `profiles` | Profili disponibili: oggetto `nome libreria trasformato → elenco di nomi di profilo` — vedi la convenzione di chiave qui sotto |
| `indiserver` | `"Y"` se il server INDI integrato è attivato, `"N"` altrimenti |
| `indidrivers` | Catalogo dei driver INDI installati: elenco di `{binary, family, label, mdpd}` |
| `indiactivedrivers` | Driver INDI attualmente avviati |
| `systemwatcher` | Carico del server, vedi più avanti |
| `files`, `folders` | Copia di `d.files.files` e `d.files.folders`, vedi [File system](#fichiers) |

{{% notice style="warning" title="Chiave di `profiles`: non il nome della libreria così com'è, non `infos.template`" %}}
`profiles` è indicizzato dal **nome della libreria** del modulo (la stessa stringa passata come `lib` a [`ML`]({{% relref "client-messages.it.md" %}}#ml), cioè la chiave in `controllerdata.libraries` senza il prefisso `libost`) — ma trasformata: **prima lettera maiuscola, il resto minuscolo** (`"focus"` → `"Focus"`, non una semplice capitalizzazione — un nome di libreria con un'altra maiuscola verrebbe comunque appiattito, es. un ipotetico `"myFocus"` → `"Myfocus"`). Questa chiave non è né il nome/etichetta dell'istanza del modulo, né necessariamente identica a `infos.template` (di solito coincidono in pratica, ma i due sono impostati indipendentemente e nulla nel protocollo lo garantisce).

Questa convenzione non è documentata altrove e non è imposta dal backend — è stata ricostruita analizzando la logica di ricerca di Rooster. Due client indipendenti costruiti solo a partire da questa documentazione ci sono cascati (un selettore di profili che si rendeva silenziosamente vuoto). Se devi cercare i profili disponibili di un modulo, riproduci esattamente questa trasformazione sul nome della libreria invece di indovinare.
{{% /notice %}}

{{% notice style="warning" title="`uc`: una chiave per messaggio, valore sostituito in blocco" %}}
Un messaggio `uc` porta **una sola chiave** di `controllerdata`, e il suo valore sostituisce **interamente** quello precedente (nessuna fusione). Un client che legge solo la prima chiave del payload (come fa Rooster) è quindi corretto finché il server invia una sola chiave per messaggio, il che oggi è vero.
{{% /notice %}}

### `systemwatcher`: carico del server

Il server misura il proprio carico a intervalli regolari e lo diffonde a tutti i client. L'intervallo si regola con l'opzione `--systemwatchinterval` (secondi, **10 per impostazione predefinita**, `0` disattiva). Il messaggio è un normale `uc`, e l'ultimo valore è presente anche in `controllerdata.systemwatcher` del dump.

```json
{
  "uc-update controller data": {
    "systemwatcher": {
      "cpu": { "load_1m": 10.42, "load_5m": 9.79, "load_15m": 4.51 },
      "ram": { "used_mb": 12194, "total_mb": 31941 },
      "disks": [
        { "device": "/dev/nvme0n1p2", "mountpoint": "/", "used_gb": 789.4, "total_gb": 915.3 }
      ],
      "network": [
        { "iface": "enp4s0", "rx_kbps": 16.27, "tx_kbps": 448.8 }
      ],
      "temperatures": [
        { "type": "Tctl", "temp_c": 75.375 }
      ]
    }
  }
}
```

| Campo | Significato |
|---|---|
| `cpu.load_1m` / `load_5m` / `load_15m` | **Carico medio** del sistema (come `/proc/loadavg`), **non una percentuale di utilizzo**: 1.0 = un core saturo in media |
| `ram.used_mb` / `total_mb` | Memoria utilizzata (totale − disponibile) e totale, in MB |
| `disks[]` | Un elemento per dispositivo di archiviazione montato (`/dev/*`, esclusi i `loop`): `device`, `mountpoint`, `used_gb`, `total_gb` |
| `network[]` | Un elemento per interfaccia (escluso `lo`): velocità `rx_kbps` / `tx_kbps` misurate tra due messaggi; la prima misura di un'interfaccia vale 0 |
| `temperatures[]` | Sensori hardware del processore: `type`, `temp_c`. **L'elenco può essere vuoto** a seconda della macchina (vengono letti solo i sensori Intel `coretemp` e AMD `k10temp`) |

{{% notice style="tip" title="Non presumere che gli array siano popolati" %}}
`disks`, `network` e soprattutto `temperatures` possono essere vuoti. Un client deve nascondere la sezione corrispondente invece di mostrare una tabella vuota. Il protocollo non fornisce il numero di core del server: un indicatore di CPU non può quindi essere una vera percentuale, il valore va mostrato come carico.
{{% /notice %}}

## Configurazioni e ciclo di vita dei moduli

Una **configurazione** è un insieme denominato di moduli (libreria, etichetta, profilo) che il server può ricaricare in un colpo solo. L'elenco si trova in `controllerdata.availableconfs`, la configurazione attiva in `controllerdata.currentconf`.

| Azione | Comando | Dettaglio |
|---|---|---|
| Caricare una configurazione | [`CL`]({{% relref "client-messages.it.md" %}}#cl) | Carica tutti i moduli della configurazione |
| Salvare la configurazione corrente | [`CS`]({{% relref "client-messages.it.md" %}}#cs) | Con il nome indicato |
| Caricare un modulo | [`ML`]({{% relref "client-messages.it.md" %}}#ml) | Libreria, etichetta, profilo facoltativo |
| Scaricare un modulo | [`MK`]({{% relref "client-messages.it.md" %}}#mk) | Per nome del modulo |

I moduli effettivamente caricati sono quelli del dump (`d.m`); compaiono e scompaiono tramite [`aa`]({{% relref "server-messages.it.md" %}}#aa) e [`dm`]({{% relref "server-messages.it.md" %}}#dm). La configurazione **non contiene mai** `GlobalDatastore` (vedi sotto), che è sempre presente.

### Server INDI e driver

Se `controllerdata.indiserver` vale `"Y"`, il server pilota un'istanza INDI integrata. I comandi [`YA`/`YZ`/`YL`/`YR`/`YS`]({{% relref "client-messages.it.md" %}}#indi) la avviano, la arrestano e gestiscono i driver; `controllerdata.indidrivers` fornisce il catalogo disponibile e `controllerdata.indiactivedrivers` ciò che è in esecuzione. `binary` è il valore da passare come `driver` in `YL`, `YR` e `YS`.

## Il modulo `GlobalDatastore` {#globaldatastore}

`GlobalDatastore` è un modulo **come gli altri nel dump** (`d.m.GlobalDatastore`, con proprietà, elementi, griglie, profili), ma il suo ruolo è centrale:

- è creato dal server **prima di tutti gli altri moduli** ed è **sempre presente**: `MK` su di esso viene rifiutato, e non è mai incluso in una configurazione;
- contiene il **riferimento condiviso dell'attrezzatura**, che tutti gli altri moduli consultano: ottiche, località, attrezzature (set di dispositivi) e server INDI.

Le sue griglie servono da sorgente per elenchi di scelta (LOV) con lo stesso nome, presenti in `d.lovs` (`optics`, `locations`, `equipments`, `servers`) e ricostruiti a ogni modifica della griglia. Gli altri moduli le usano così: scegliere un'ottica compila la focale e il diametro, scegliere una località compila latitudine, longitudine e altitudine, scegliere un set di attrezzature risolve i dispositivi (camera, montatura, focheggiatore, ruota portafiltri, camera di guida, GPS, cupola, meteo) e il server INDI da contattare.

Proprietà esposte:

| Proprietà | Ruolo |
|---|---|
| `devices` | Set di attrezzature e GPS selezionati (`equipments`, `gps`) |
| `devicesactions` | Pulsanti: connettere/disconnettere i dispositivi, caricare le configurazioni (`condevs`, `discondevs`, `loadconfs`) |
| `server` | Host e porta del server INDI (`host`, `port`) |
| `serveractions` | Pulsanti: connettere/disconnettere il server INDI (`conserv`, `disconserv`) |
| `startup` | Caselle di avvio automatico (`indiatstart`, `devatstart`, `confsatstart`) |
| `equipments` | Griglia dei set di attrezzature |
| `locations` | Griglia delle località (icone `refresh` = leggere il GPS, `add` = aggiungere la riga corrente) |
| `optics` | Griglia delle ottiche |
| `servers` | Griglia dei server INDI |

Tutte le sue proprietà si manipolano con i messaggi standard (`SV`, `SA`, `GC`…, `I3`/`I4` su `locations`); un client generico le mostra come qualsiasi altro modulo. Rooster le presenta in una scheda dedicata della sua pagina di parametri, mantenendo le proprietà che hanno un profilo (`hasprofile`) e raggruppandole per `level1`/`level2`.

## File system {#fichiers}

Il server pubblica l'elenco dei file prodotti (immagini, sequenze, archivi) nella cartella `--webroot`, senza che il client debba interrogarlo. Questo meccanismo è indipendente dai moduli.

### Nel dump: `d.files`

```json
"files": {
  "folders": ["", "/Allsky", "/Sequencer/gam Cyg/LIGHT/Luminance"],
  "files": ["/image1.jpeg", "/rapport.txt"],
  "selectedfolder": "/home/gilles/ostmedia"
}
```

| Campo | Contenuto |
|---|---|
| `folders` | **Tutte** le sottocartelle del webroot, ricorsivamente, percorsi relativi al webroot che iniziano con `/`. La radice è la stringa vuota `""`. Non ordinato. |
| `files` | I file **della sola cartella selezionata** (non ricorsivo), percorsi relativi al webroot che iniziano con `/`. Tutti i file, senza filtro di estensione. |
| `selectedfolder` | La cartella selezionata, come **percorso assoluto del file system del server** (e non relativo). |

Non ci sono né paginazione né limite di dimensione: su un webroot molto pieno l'elenco può essere lungo.

### Scegliere la cartella: `FS`

[`FS`]({{% relref "client-messages.it.md" %}}#fs) indica la cartella da sorvegliare e di cui elencare i file. Il server sorveglia il **contenuto** di una sola cartella alla volta; l'albero delle cartelle (`folders`) è invece tracciato in modo indipendente, vedi sotto.

{{% notice style="warning" title="La cartella selezionata è globale per il server" %}}
Non esiste una cartella per client: un `FS` cambia l'elenco di file visto da **tutti** i client connessi. Un client non deve usarlo per una semplice navigazione «locale» se altri client possono essere connessi.

Il percorso **viene validato**: la destinazione deve esistere e trovarsi all'interno del webroot (nessuna via di fuga con `../`). Una richiesta non valida viene rifiutata lato server (registrata, senza cambiamento di stato) — non restituisce silenziosamente un elenco vuoto.
{{% /notice %}}

Un `FS` riuscito sostituisce l'elenco di file in blocco: il client riceve un solo messaggio [`uc`]({{% relref "server-messages.it.md" %}}#uc) `files` con l'elenco completo della nuova cartella, non un flusso di `af`/`df` — selezionare una cartella non è una mutazione del file system, non è stato realmente aggiunto o rimosso nulla. **Non** c'è alcun messaggio che annuncia il nuovo `selectedfolder` in sé, che esiste solo nel dump.

### Albero delle cartelle vs. file della cartella selezionata

`folders` (l'intero albero ricorsivo) e `files` (il contenuto della cartella selezionata) non hanno lo stesso ambito:

| | Ambito | Aggiornamenti live |
|---|---|---|
| `folders` / `ad` / `dd` | **Tutto il webroot**, ricorsivamente — indipendente da cosa è selezionato | Sempre, indipendentemente dalla selezione `FS` corrente |
| `files` / `af` / `df` | **Solo la cartella selezionata** | Solo per i cambiamenti alla cartella selezionata dall'ultimo `FS` |

Una cartella creata o eliminata ovunque nel webroot viene segnalata live tramite `ad`/`dd` anche se nessun client l'ha attualmente selezionata. Un file creato o eliminato viene segnalato tramite `af`/`df` solo se riguarda la cartella selezionata per ultima dal `FS` di un client — `files`/`af`/`df` non dicono nulla sul contenuto delle altre cartelle, un client deve farci un `FS` per scoprirlo.

### Aggiornamenti: `af` / `df` / `ad` / `dd`

Quando l'albero delle cartelle o il contenuto della cartella selezionata cambia realmente, il server invia un messaggio per ogni percorso interessato — il valore è il percorso nudo stesso, non racchiuso in un oggetto o array:

```json
{ "af": "/Sequencer/gam Cyg/LIGHT/Luminance/img_001.FITS" }
{ "df": "/rapport.txt" }
{ "ad": "/Sequencer/nuovo target" }
```

| Chiave | Significato |
|---|---|
| `af` | File apparso nella cartella selezionata |
| `df` | File scomparso dalla cartella selezionata |
| `ad` | Sottocartella apparsa ovunque nel webroot |
| `dd` | Sottocartella scomparsa ovunque nel webroot |

Ogni messaggio porta esattamente un percorso (N file = N messaggi) — vedi [`af`/`df`/`ad`/`dd`]({{% relref "server-messages.it.md" %}}#af) per il riferimento completo. Una rinomina viene segnalata come un'eliminazione seguita da una creazione: il server confronta un elenco prima/dopo, non rileva le rinomine in quanto tali.

Dopo i messaggi per percorso, il server invia [`uc`]({{% relref "server-messages.it.md" %}}#uc) `files` e/o `folders` con l'elenco completo aggiornato — ma solo per quello dei due che è realmente cambiato in questo passaggio, mai incondizionatamente. Un client può quindi applicare gli eventi per percorso in modo incrementale, oppure semplicemente sostituire il/i proprio/i elenco/i con il/i messaggio/i `uc` corrispondente/i.

### Costruire l'URL di un file

I percorsi di `files` sono relativi al webroot e iniziano con `/`. L'URL di un file è l'URL di base dei media del server (`http(s)://hostname/ostmedia/`, vedi la nota « URL media » del [Modello dati]({{% relref "structure.it.md" %}})) seguita dal percorso. Attenzione alla `/` iniziale: concatenarla a una base che termina già con `/` dà una doppia `//`.

### Ciò che l'evento non fa

Il server rileva l'**aggiunta, l'eliminazione e la ridenominazione** di file, non la modifica del contenuto di un file esistente: un'immagine riscritta con lo stesso nome non produce alcun evento. È il caso delle immagini di anteprima dei moduli, il cui URL resta identico a ogni ripresa: un client deve aggiungere un parametro di cache-busting (`?t=` + marca temporale) quando ricarica un'immagine dopo un aggiornamento della sua proprietà.
