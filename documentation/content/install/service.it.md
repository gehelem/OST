---
title: Gestire il servizio ostserver
weight: 30
description: "Avviare, arrestare, personalizzare le opzioni di avvio del servizio ostserver"
---

`ostserver` funziona in background come **servizio systemd**. Systemd è il gestore di servizi di Linux: avvia automaticamente `ostserver` all'avvio della macchina, lo riavvia se si blocca, e permette di controllarlo con alcuni comandi semplici.

---

## Avviare, arrestare, riavviare

```bash
# Avviare il servizio
sudo systemctl start ostserver

# Arrestare il servizio
sudo systemctl stop ostserver

# Riavviare il servizio (utile dopo aver modificato le opzioni)
sudo systemctl restart ostserver

# Vedere lo stato del servizio
sudo systemctl status ostserver
```

---

## Attivare / disattivare l'avvio automatico

Di default, il servizio è **attivato**: si avvia automaticamente a ogni avvio della macchina.

```bash
# Disattivare l'avvio automatico
sudo systemctl disable ostserver

# Riattivare l'avvio automatico
sudo systemctl enable ostserver
```

---

## Consultare i log

```bash
# Mostrare gli ultimi log
sudo journalctl -u ostserver

# Seguire i log in tempo reale
sudo journalctl -u ostserver -f

# Limitare alle ultime 100 righe
sudo journalctl -u ostserver -n 100
```

---

## Personalizzare le opzioni di avvio

Durante l'installazione, viene creato un file di configurazione nella propria cartella utente:

```
~/.ost/ostserver.parms
```

Questo file contiene **un argomento per riga**. Le righe che iniziano con `#` sono commenti e vengono ignorate. Gli spazi nei valori sono supportati nativamente — non è necessario alcun virgolettamento.

Per modificarlo:

```bash
nano ~/.ost/ostserver.parms
```

Esempio di contenuto:

```
# Web server media folder (must be writable)
--webroot=/var/lib/osterix/media

# Start embedded INDI server (Y/N)
--indiserver=Y

# Log level: 0=debug, 1=info, 2=warning, 3=error, 4=critical
--loglevel=1

# Security level: 0=disabled, 1=read only, 2=user grants
--grant=0

# Frontend banner text (spaces are allowed, no quoting needed)
--banner=Il mio osservatorio senza testa
```

Dopo qualsiasi modifica, riavviare il servizio perché sia applicata:

```bash
sudo systemctl restart ostserver
```

{{% notice style="info" title="File preservato durante gli aggiornamenti" %}}
Il file `ostserver.parms` non viene **mai sovrascritto** durante un aggiornamento del pacchetto. Le personalizzazioni vengono conservate.
{{% /notice %}}

---

## Riferimento delle opzioni disponibili

| Opzione | Predefinito | Descrizione |
|---|---|---|
| `--webroot=<percorso>` | `/var/lib/osterix/media` | Cartella media servita da nginx |
| `--indiserver=Y\|N` | `N` | Avviare il server INDI integrato |
| `--loglevel=<0-4>` | `1` | Verbosità dei log: 0=debug, 1=info, 2=warning, 3=error, 4=critical |
| `--logfile=<percorso>` | *(con marca temporale in `~/.ost/`)* | Percorso del file di log |
| `--grant=<0\|1\|2>` | — | Sicurezza: 0=disattivata, 1=solo lettura, 2=grant utente |
| `--lng=<fr\|en\|...>` | `fr` | Lingua dell'interfaccia (fr, en, de, it, es) |
| `--banner=<testo>` | `Observatoire Sans tête` | Banner mostrato nel frontend |
| `--configuration=<nome>` | `default` | Nome della configurazione da caricare all'avvio |
| `--libpath=<percorso>` | *(percorso di sistema)* | Cartella dei moduli `.so` |
| `--dbpath=<percorso>` | *(cartella di lavoro)* | Posizione del database |
| `--systemwatchinterval=<s>` | `10` | Intervallo di monitoraggio del sistema in secondi (0 per disattivare) |
| `--minfree=<0-99>` | `10` | Spazio disco libero minimo in % prima di saltare la scrittura di un'immagine |
| `--ssl=Y\|N` | `N` | Attivare SSL per il WebSocket |
| `--sslcert=<percorso>` | `/etc/ost/server.crt` | Certificato SSL |
| `--sslkey=<percorso>` | `/etc/ost/server.key` | Chiave privata SSL |
| `--setadminpassword=<password>` | — | Impostare la password amministratore (da rimuovere dopo l'uso) |
