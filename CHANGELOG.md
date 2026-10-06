## 0.0.15 (2026-10-06)

### fixed (16 changes)

- [ost: stop baking runtime values into translatable startup log lines](https://gitlab.ostserver.fr/ost/ostserver/-/commit/4bb510b292039f3832d75c6a6c5c1bfff0ecc2e7) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [focus: tune StellarSolver detection instead of the raw "1-Default" profile](https://gitlab.ostserver.fr/ost/ostserver/-/commit/792e7f7401c68bdddfd51606a4047c2aea732563) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [inspector: tunable donut-detection sensitivity, reject point sources](https://gitlab.ostserver.fr/ost/ostserver/-/commit/8ed3cd8193b21705789270bafc8dc68cfb1a5cd1) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [inspector: bound the collimation analysis on flat/saturated frames](https://gitlab.ostserver.fr/ost/ostserver/-/commit/f3319151484aa7561fd5a8d94f311d4b8bb2fb10) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [parkmanager: fix crash when a configured device isn't actually reachable](https://gitlab.ostserver.fr/ost/ostserver/-/commit/86500f3c6f7a5f631e1cadfa90534aacb8f65112) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Fix RA wraparound bug in navigator centering](https://gitlab.ostserver.fr/ost/ostserver/-/commit/b511271a2982a08d7112bae9b4bc7e5873cf7897) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Docs: fix type/template/family confusion, document loadedFamilies-<X>](https://gitlab.ostserver.fr/ost/ostserver/-/commit/ad075e9483193f99b898435041888f81b0ed0795) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Fix infos.classname/library/families: compute at dump time, not in ctor](https://gitlab.ostserver.fr/ost/ostserver/-/commit/e25901c562b9f1f0055b64805293a9b4a2dba239) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Fix PropertyMulti::setElt() silently dropping light-type writes](https://gitlab.ostserver.fr/ost/ostserver/-/commit/c416ef5718f4d244975b4b8fc3fe8e626b1315bb) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Controller: actually send loadedModules-<template>/profiles-<Type> lovs](https://gitlab.ostserver.fr/ost/ostserver/-/commit/8ac7aab785828c90c1a213885444fd188c379ff4) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Fix several doc/code inconsistencies found via ostharfang3/ostharfang4](https://gitlab.ostserver.fr/ost/ostserver/-/commit/3da7368d01ca392732ee688855f1d83aea2b1728) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Fix Sequencer's template metadata: "sequence" -> "sequencer"](https://gitlab.ostserver.fr/ost/ostserver/-/commit/b86e8db360511f7e634ff9ef6d4704499b5f56e7) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Make the shared imager "image" property read-only, not write-only](https://gitlab.ostserver.fr/ost/ostserver/-/commit/1ae4fdbad7de5095d71a4ff2dac80be0bec4b506) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Ignore light element writes instead of requiring/applying them](https://gitlab.ostserver.fr/ost/ostserver/-/commit/64107c69511e854930eaf47bc431ae6637de16df) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Keep folder tree tracking global, scope file tracking to selected folder](https://gitlab.ostserver.fr/ost/ostserver/-/commit/c258558ac78324340e2379a013792b9faff0faf2) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Fix redundant/duplicate messages around file/folder watching](https://gitlab.ostserver.fr/ost/ostserver/-/commit/8bbaf6fe81cfd92d3d72a58490f1b10c1d8116e8) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))

### added (9 changes)

- [i18n: merge pending translations (fr/en/de/es/it)](https://gitlab.ostserver.fr/ost/ostserver/-/commit/42b8a42cb4d21b7bab0d356f35942fd60cb1a015) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [docs(focus): document the new star-detection tuning parameters](https://gitlab.ostserver.fr/ost/ostserver/-/commit/4aabeaff1cab2b820062529b15837b487a76229b) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [docs(inspector): document the new collimation-detection parameters](https://gitlab.ostserver.fr/ost/ostserver/-/commit/166030758d345d7d3818f83b4bb433c74c82f048) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Sequencer: log the real cause of a Sequence error, not just the fact](https://gitlab.ostserver.fr/ost/ostserver/-/commit/da5d307f3261981ef3b5f7342e9367d3ee69583c) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Add the focus family, switch Sequencer/Monitor slave slots to it](https://gitlab.ostserver.fr/ost/ostserver/-/commit/c8d3a5171f0fb409294ce1535f2a6ce7d6e3e2ec) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Add a generic module family concept, wire up guiding](https://gitlab.ostserver.fr/ost/ostserver/-/commit/db65e223c940eeb08eb0b00d0212aa39eb4d1ec9) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Document the profiles keying convention](https://gitlab.ostserver.fr/ost/ostserver/-/commit/45972bd895877dcc1cf6a58bb4ffa015c9044cef) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Add lovscopes to dummy: one element per LOV scope (local/module/controller)](https://gitlab.ostserver.fr/ost/ostserver/-/commit/ce91b22cc18d192bd02550f9cf16dc5a7f2bd543) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/249))
- [Document controller data, GlobalDatastore and the file system](https://gitlab.ostserver.fr/ost/ostserver/-/commit/801d0de02d3603692556553b62baf216de627fa5) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/249))

### changed (4 changes)

- [Docs: clarify grant-server access policy values](https://gitlab.ostserver.fr/ost/ostserver/-/commit/45362c3f0e6e0aa645b0d272ff27f0ea347af6e8) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Harmonize slave-facing action commands into shared defineMeAsX](https://gitlab.ostserver.fr/ost/ostserver/-/commit/de9603660bffdf5b60db2152ae7b6f0c7b64129d) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Replace evt/fileevent file events with af/df/ad/dd, bare-string payload](https://gitlab.ostserver.fr/ost/ostserver/-/commit/d6c8b1ca1685512196806f9675f0c575a2310b65) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Load all star catalogs unconditionally at DB init, remove popcat](https://gitlab.ostserver.fr/ost/ostserver/-/commit/83512cc807948a8eb50f91996fcb02133b66b53f) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))

### other (12 changes)

- [Restore QY/qa docs, reframed as inter-module communication](https://gitlab.ostserver.fr/ost/ostserver/-/commit/58509500f2d7b66c26a7f089104c62c44279f29c) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Reframe the GC multi-ea notice: apply each message, don't reason about it](https://gitlab.ostserver.fr/ost/ostserver/-/commit/69c68fb73ac90e9b54bc508da1c5a46930fbf4a8) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Remove QY/qa from the client-facing protocol docs](https://gitlab.ostserver.fr/ost/ostserver/-/commit/ac2c1884b682ba5b7d4c2fd2c43b2f6c412cdef0) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Clarify controllerdata.libraries keys are libost-prefixed, ML rejects double prefixing](https://gitlab.ostserver.fr/ost/ostserver/-/commit/d7afe0e7fda12882bfedfe931b244bcdf5727da9) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Recommend supporting at least a dark and a light theme](https://gitlab.ostserver.fr/ost/ostserver/-/commit/bf720485612546e3a50665a678ce09be53987b13) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Document icon actions, offline requirement, QY/qa, live graph rendering](https://gitlab.ostserver.fr/ost/ostserver/-/commit/6a5d152a3b6f02c4af39c080d59bf6856adbe670) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Document property forms, WS URL convention, graph types, configuration internals](https://gitlab.ostserver.fr/ost/ostserver/-/commit/2551ac77d53705bee34d362fbfb2953cfa2b2b25) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Document DP, GR, mDNS discovery, INDI ack behavior and indiactivedrivers shape](https://gitlab.ostserver.fr/ost/ostserver/-/commit/6978baf77dc1045f3ba8a4c538e0fe1d20dbbe28) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Document the global folder tree vs. scoped file tracking, 5 languages](https://gitlab.ostserver.fr/ost/ostserver/-/commit/af35142ed1a5b61771c2915c7b21f62db3d6e9ee) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Document the af/df/ad/dd file event format, drop the evt/ev pitfall note](https://gitlab.ostserver.fr/ost/ostserver/-/commit/89bd0d34c90cc58beac0c9e556bdce2220cc0aa3) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Parse CLI arguments before constructing QGuiApplication](https://gitlab.ostserver.fr/ost/ostserver/-/commit/aa7edea5a42582c5606557684ce916ef3a76e289) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))
- [Reorder dev-client documentation pages and document module templates](https://gitlab.ostserver.fr/ost/ostserver/-/commit/1cd2b193519a0f01496fcf42ee06e6cad4855079) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/250))

## 0.0.14 (2026-09-20)

### other (1 change)

- [Document directedit enforcement and the independent-bool principle](https://gitlab.ostserver.fr/ost/ostserver/-/commit/53da9fe9a666c5704bd23a427dbdd010c2187977) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/248))

## 0.0.12 (2026-09-19)

### fixed (2 changes)

- [Fix img element fields duplicated flat AND nested under value](https://gitlab.ostserver.fr/ost/ostserver/-/commit/aa5e9b4a6ca82f700899de1b3b3fbf0a3d3980d6) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/245))
- [Fix lc events for module-JSON-declared globallovs sent with an empty key](https://gitlab.ostserver.fr/ost/ostserver/-/commit/ed5edf15c052dfd8fbf03856db5f5f2a7cda0f39) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/244))

### other (6 changes)

- [doc: add security posture warning (local network / VPN, not for direct internet exposure)](https://gitlab.ostserver.fr/ost/ostserver/-/commit/661324d73661c851d68c499dbca378e0ed64e838) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/243))
- [doc: clarify that ee is backend-initiated only, never a response to SV/SA](https://gitlab.ostserver.fr/ost/ostserver/-/commit/31c43daf5c6a9cf4c7b67a8b60ddc4d25e1df67c) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/243))
- [doc: fix infos.modulelabel field name, clarify directedit semantics, document...](https://gitlab.ostserver.fr/ost/ostserver/-/commit/9fd173c07121ca4544d39a143355947f99a0ba0f) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/243))
- [doc: clarify directedit omission, nullable/lovConstrained on string, SA...](https://gitlab.ostserver.fr/ost/ostserver/-/commit/57e6e311cfe0f601d9604f3238cae48d6138443b) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/243))
- [doc: explain img element per-channel stats and histogram bin structure](https://gitlab.ostserver.fr/ost/ostserver/-/commit/9794ebdb2dfa033a5d1d9e1e50c84f6c6ae6b73b) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/243))
- [doc: clarify media URLs are served on a separate host/port from the WebSocket](https://gitlab.ostserver.fr/ost/ostserver/-/commit/e7c1f713d6dff3f4258a16c70c0eb9da81443c98) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/243))

### added (9 changes)

- [doc: add Italian, German and Spanish translations](https://gitlab.ostserver.fr/ost/ostserver/-/commit/b2bcfe26fb58f1aef28d09f59d618737cbb7f307) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/242))
- [Add a minimal framework-free example from test-ws-advanced.js](https://gitlab.ostserver.fr/ost/ostserver/-/commit/b59799ed110ced674dfd053ec1cc3ad72272f0eb) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/241))
- [Add real read/write code examples from OstErix](https://gitlab.ostserver.fr/ost/ostserver/-/commit/d020a3f56dc3d6ddc7d71d5c21e152c7d4bc79db) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/241))
- [Simplify the read/write diagram](https://gitlab.ostserver.fr/ost/ostserver/-/commit/f218c12be93e8fa706260eca0e1c9668559ce809) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/241))
- [Document reconnection: full datastore reset, DU is nearly mandatory](https://gitlab.ostserver.fr/ost/ostserver/-/commit/0c453c7abe65e6a2ecf1f5fb1a7678234f780521) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/241))
- [Rename title to 'Principes d'architecture client' - fix the anglicism](https://gitlab.ostserver.fr/ost/ostserver/-/commit/fce777d109694bd7ff91710e71ca7e6345704bda) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/241))
- [Fill in heartbeat mechanism, cite ev, drop the wrong 'messages undocumented' TODO](https://gitlab.ostserver.fr/ost/ostserver/-/commit/a0127a1db1f2043c7673935736fea7a665d7b690) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/241))
- [Clarify heartbeat is optional, client-side only](https://gitlab.ostserver.fr/ost/ostserver/-/commit/46cc0aeb034260e458b08a950850b230fb29365c) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/241))
- [Document the client architecture pattern (single datastore, WS-only writes)](https://gitlab.ostserver.fr/ost/ostserver/-/commit/a66b955773755580a126612627f367e9339666e1) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/241))

## 0.0.11 (2026-09-18)

### added (1 change)

- [Replace the changelog API POST with a real git push](https://gitlab.ostserver.fr/ost/ostserver/-/commit/a0b9f781406feb83c998f0388b527afa523ec0db) ([merge request](https://gitlab.ostserver.fr/ost/ostserver/-/merge_requests/240))
