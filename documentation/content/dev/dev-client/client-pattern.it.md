---
title: Principi di architettura client
weight: 10
description: "Come strutturare un client OST: datastore unico, scrittura solo via WS"
---

Questa pagina descrive **come deve essere strutturato un client**, non il formato
dei messaggi scambiati (vedi [Modello dati]({{% relref "structure.it.md" %}})
e le pagine seguenti per quello).

## Principio: un datastore unico detiene la verità

Il client mantiene **un'unica struttura di dati** che rappresenta lo stato
corrente (moduli → proprietà → elementi). Tutta la UI **legge** questa
struttura; nulla altro la modifica se non i messaggi ricevuti dal server.

```
Lettura   :  Datastore ────────────► UI

Scrittura :  UI ─── SV/SA ───► Server ─── update ───► Datastore
```

- **Nessuna duplicazione**: se un componente ha bisogno di un valore, lo legge
  dal datastore, non lo conserva per proprio conto.
- **Nessuna scrittura diretta**: la UI non modifica mai il datastore da sola,
  nemmeno per una visualizzazione ottimistica.

{{% notice style="tip" title="Scrittura: sempre tramite il server" %}}
Un'azione dell'utente (cambiare un valore, cliccare un pulsante) attiva
l'invio di un messaggio al server (`SV`/`SA` — vedi
[Client → Server]({{% relref "client-messages.it.md" %}})). Il datastore
locale **cambia solo in risposta**, quando il server risponde con
l'aggiornamento confermato. Il client non presume mai il risultato.
{{% /notice %}}

## Dump completo vs aggiornamenti parziali

Alla connessione, il client invia `DU`; il server risponde con un dump
completo (`d`) contenente tutti i moduli con i loro metadati completi
(label, tipo, min/max, LOV, ecc.).

{{% notice style="tip" title="Gli aggiornamenti parziali portano solo il valore" %}}
I messaggi che arrivano successivamente (`ee`, `ea`) contengono **solo il
valore**, non i metadati. Il client deve:
1. Conservare i metadati dell'ultimo dump completo.
2. Su un aggiornamento parziale, applicare la patch **solo** al valore, senza
   mai supporre che i metadati siano cambiati.

Se i metadati stessi cambiano (min/max/formato di un elemento), il server
lo segnala esplicitamente tramite un messaggio distinto, `ev`
(vedi [Server → Client]({{% relref "server-messages.it.md" %}})) — questo
non è mai silenzioso. Il client deve quindi solo reagire al tipo di
messaggio ricevuto, senza doverla indovinare cosa è cambiato.
{{% /notice %}}

{{% notice style="warning" title="Trappola conosciuta: gli aggiornamenti di GlobalLOV" %}}
Quando una proprietà fa riferimento a una LOV globale e questa cambia, il
server invia l'aggiornamento **senza il campo `value`**. Il client non deve
**mai preservare il vecchio valore** in questo caso preciso — bisogna
applicare esattamente ciò che il server invia, inclusa l'assenza di
valore. Il server fa fede; non tentare mai di "riempire" un campo
mancante con il vecchio stato locale.
{{% /notice %}}

## Rendering: limitare gli aggiornamenti alla proprietà modificata

{{% notice style="warning" title="Obbligatorio: mai un re-render più ampio della proprietà modificata" %}}
Alcuni moduli possono inviare più aggiornamenti `ee`/`ea` **al
secondo** (ad esempio un modulo che interroga continuamente un
dispositivo). Un client che reagisce a ogni aggiornamento in arrivo
ridisegnando tutta la sua UI — o anche solo l'intero modulo
visualizzato — diventa inutilizzabile sotto questo carico: perdita del
focus di digitazione, sfarfallio, e nel peggiore dei casi l'UI smette
del tutto di rispondere agli input.

Non è un caso limite da gestire "per precauzione": è un comportamento
reale e attuale del backend, e i client **devono** essere costruiti
per gestirlo fin dall'inizio:
1. Ad ogni aggiornamento, deve essere sostituito/mutato solo l'oggetto
   JS/dati della proprietà modificata. Il riferimento di ogni altra
   proprietà deve rimanere esattamente identico finché non è
   coinvolta.
2. Il rendering della UI deve essere indicizzato sull'identità della
   proprietà (o del modulo) — un elenco di proprietà dovrebbe
   riutilizzare lo stesso elemento UI per la stessa proprietà tra un
   rendering e l'altro, senza mai distruggerlo e ricrearlo, così un
   campo non coinvolto non viene mai toccato, qualunque cosa succeda
   altrove.
3. Limitare solo la **frequenza** dei rendering (debounce/throttle)
   non basta se il rendering stesso continua a ricostruire UI non
   coinvolta — la soluzione sta nel **perimetro** di ciò che viene
   ricostruito, non solo nella sua frequenza.

Rooster (il frontend di produzione) implementa questo principio per
costruzione: un aggiornamento in arrivo sostituisce solo il
riferimento della proprietà coinvolta nello store, e ogni elenco di
proprietà viene renderizzato con una chiave stabile per proprietà. Il
risultato è che la UI delle proprietà non coinvolte non viene mai
disturbata, indipendentemente dalla velocità con cui un'altra
proprietà si aggiorna altrove.
{{% /notice %}}

## Template: schermate per tipo di modulo con fallback generico

Ogni istanza di modulo può dichiarare `infos.template` (vedi [Modello
dati]({{% relref "structure.it.md" %}})) — una stringa che ne indica il
tipo (`"focus"`, `"guider"`, `"sequencer"`...). È solo un suggerimento,
ed **è ammesso che sia assente**.

{{% notice style="tip" title="Template assente = la schermata di default" %}}
Un client che supporta schermate personalizzate per template dovrebbe
trattare un `infos.template` assente (o non riconosciuto) esattamente
come il valore letterale `"default"` — non come un errore. Rooster fa
così con un piccolo registro che associa un nome di template in
minuscolo a un componente di schermata, ricadendo sulla propria
schermata generica/di default quando il nome è assente o non presente
nel registro:
```
template = infos.template?.toLowerCase() || "default"
screen = registry[template] ?? registry["default"]
```
Questo è ciò che permette a un client di restare pienamente funzionante
di fronte a un tipo di modulo mai visto prima — un modulo nuovo o di
terze parti viene sempre renderizzato tramite la schermata generica —
pur offrendo una schermata curata e mirata per i tipi di modulo che
meritano l'investimento.
{{% /notice %}}

{{% notice style="tip" title="Consigliato: un unico componente di rendering delle proprietà, riusato ovunque" %}}
Non obbligatorio, ma vale decisamente la pena: costruire un **unico**
componente responsabile del rendering di una proprietà (o di un
elemento) — gestendo `directedit`, `permission`, le tre portate di LOV,
le regole dei gruppi bool, `autoupdate`, le griglie, i grafici — e far
sì che **ogni** schermata lo usi per le singole proprietà, comprese le
schermate personalizzate per template, non solo il fallback generico.

L'alternativa — ogni schermata che reimplementa il proprio rendering
dei campi — duplica ogni sfumatura del formato di trasporto in tutto il
codice, e una correzione o una nuova funzionalità del backend raggiunge
solo le schermate che qualcuno si ricorda di aggiornare. Le schermate
personalizzate di Rooster (`GuiderComponent`, `FocusComponent`...)
renderizzano il proprio layout, ma delegano comunque ogni singola
proprietà allo stesso componente di proprietà generico usato dalla
schermata di fallback — una correzione fatta lì una sola volta (ad
esempio la corretta gestione di un gruppo di switch esclusivi) si
applica automaticamente ovunque, schermate personalizzate comprese.
{{% /notice %}}

## Heartbeat

{{% notice style="note" title="Facoltativo" %}}
Il heartbeat non è imposto dal protocollo. È un meccanismo che il client
mette in atto per proprio conto, solo se la sua tecnologia lo richiede,
per mantenere attiva la connessione WebSocket (alcuni browser, proxy o
piattaforme chiudono una connessione ritenuta inattiva). Un client che
non ne ha bisogno può farne completamente a meno.

Il meccanismo esiste a livello di protocollo per chi ne ha bisogno:
inviare `XX` ogni 30 secondi, il server risponde `xx` (senza payload
utile).
```json
→ {"XX": {}}
← {"xx": {}}
```
{{% /notice %}}

## Riconnessione

{{% notice style="tip" title="Ripartire sempre da un dump completo" %}}
Alla minima interruzione, il client deve **svuotare interamente il suo
datastore** — nessuno stato parziale può essere considerato affidabile
dopo una disconnessione.

Ciò che il client fa dopo (ritentare automaticamente N volte, proporre
all'utente di riavviare la connessione, rinunciare...) riguarda solo lui:
non è specificato dal protocollo, ogni client è libero della propria
strategia.

Ma una volta (ri)connesso, rinviare `DU` per ottenere un nuovo dump
completo non è quasi opzionale: ricostruire uno stato utilizzabile a
partire dai soli aggiornamenti parziali che arriverebbero in seguito
(`ee`/`ea`/`ev`) è impossibile — questi danno solo una vista
frammentaria, mai uno stato completo. Sul lato server, nulla deve
essere preservato tra due connessioni dello stesso client: è un vincolo
puramente lato client.
{{% /notice %}}

## Esempio minimo: primo scambio

Senza framework, solo il primissimo scambio (connessione → `DU` →
ascolto) — estratto da `test-ws-advanced.js`, lo script usato per il
health-check orario del server:

```javascript
const ws = new WebSocket("ws://127.0.0.1:9624");

ws.on("open", () => {
  ws.send(JSON.stringify({ "DU": { "language": "en" } }));
});

ws.on("message", (data) => {
  console.log("received:", data.toString());
});
```

## Esempio: lettura e scrittura in Rooster

Estratti reali che illustrano la separazione lettura/scrittura vista in
cima a questa pagina, una volta che un client reale (qui Rooster, in
Angular) è costruito attorno a questo principio.

**Lettura** — `datastore.service.ts`, un semplice accesso allo stato interno:
```typescript
getProperty(moduleName: string, propertyName: string): Observable<any> {
  return new Observable(subscriber => {
    const subscription = this.changed$.subscribe(() => {
      const module = this.modules[moduleName];
      const prop = (module as any)?.properties?.[propertyName] || null;
      subscriber.next(prop);
    });
    subscriber.next(this.modules[moduleName]?.properties?.[propertyName] || null);
    return () => subscription.unsubscribe();
  });
}
```

**Scrittura** — `websocket.service.ts`, nessuna mutazione locale, solo
l'invio del messaggio:
```typescript
setPropertyOneElement(module: string, property: string, elements: { [key: string]: any }): void {
  this.send({
    SV: { m: { [module]: { p: { [property]: { e: elements } } } } }
  });
}
```
