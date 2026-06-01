--DROP TABLE IF EXISTS PROFILES;
CREATE TABLE IF NOT EXISTS PROFILES (
        MODULETYPE      TEXT NOT NULL,
        PROFILENAME     TEXT NOT NULL,
        ALLVALUES       BLOB,
        PRIMARY KEY("MODULETYPE","PROFILENAME")
);
DELETE FROM PROFILES WHERE PROFILENAME in('demo','LRGB-60s');
INSERT INTO PROFILES VALUES('GlobalDatastore','demo','{"p":{"equipments":{"e":{"camera":{"value":"CCD Simulator"},"filter":{"value":""},"focuser":{"value":""},"gps":{"value":"GPS Simulator"},"guidecamera":{"value":""},"guidest4":{"value":""},"host":{"value":"localhost:7624"},"mount":{"value":"Telescope Simulator"},"name":{"value":"Simulators light"}},"grid":[["CCD Simulator","Filter Simulator","Focuser Simulator","GPS Simulator","Guide Simulator","Telescope Simulator","localhost:7624","Telescope Simulator","Simulators"],["CCD Simulator","","","GPS Simulator","","","localhost:7624","Telescope Simulator","Simulators light"]],"gridheaders":["camera","filter","focuser","gps","guidecamera","guidest4","host","mount","name"]},"optics":{"e":{"diameter":{"format":"9999.9","max":0,"min":0,"slider":0,"step":0,"type":"float","value":50},"focal":{"format":"99999.9","max":0,"min":0,"slider":0,"step":0,"type":"float","value":242},"name":{"value":"Evoguide50"},"red":{"format":"9999.9","max":0,"min":0,"slider":0,"step":0,"type":"float","value":1},"type":{"value":"ED"}},"grid":[[250,2000,"RC10",1,"RC"],[150,600,"N150",1,"Newton"],[50,242,"Evoguide50",1,"ED"]],"gridheaders":["diameter","focal","name","red","type"]},"server":{"e":{"host":{"value":"localhost"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}}},"servers":{"e":{"host":{"value":"localhost"},"name":{"value":"localhost:7624"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}},"grid":[["localhost","localhost:7624",7624]],"gridheaders":["host","name","port"]},"startup":{"e":{"confsatstart":{"value":true},"devatstart":{"value":true},"indiatstart":{"value":true}}}}}');
INSERT INTO PROFILES VALUES('Sequencer','LRGB-60s','{"p":{"devices":{"e":{"camera":{"value":"CCD Simulator"},"equipments":{"value":"Simulators"},"filter":{"value":"Filter Simulator"}}},"optic":{"e":{"diam":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":150},"fl":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":600},"optic":{"value":"N150"},"red":{"format":"","max":10,"min":0.1,"slider":0,"step":0,"type":"float","value":1}}},"parameters":{"e":{"autofocusatstart":{"value":true},"autofocusonfilterchange":{"value":true},"ditherevery":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":0},"guidingsettletime":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":10},"hfrthreshold":{"format":"9.9","max":0,"min":0,"slider":0,"step":0,"type":"float","value":0},"rmsthreshold":{"format":"9.9","max":0,"min":0,"slider":0,"step":0,"type":"float","value":0},"suspendguidingduringfocus":{"value":true},"useguiding":{"value":true}}},"parms":{"e":{"exposure":{"format":"","max":300,"min":1e-05,"slider":2,"step":0.001,"type":"float","value":0},"gain":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":0},"offset":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":0}}},"sequence":{"e":{"count":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":10},"exposure":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":60},"filter":{"value":"3"},"frametype":{"value":"L"},"gain":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":100},"offset":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":100},"progress":{"value":{"dynlabel":"","value":0}}},"grid":[[10,60,"8","L",100,100,{"dynlabel":"","value":0}],[10,60,"1","L",100,100,{"dynlabel":"","value":0}],[10,60,"2","L",100,100,{"dynlabel":"","value":0}],[10,60,"3","L",100,100,{"dynlabel":"","value":0}]],"gridheaders":["count","exposure","filter","frametype","gain","offset","progress"]},"server":{"e":{"host":{"value":"localhost"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}}},"slaves":{"e":{"focusmodule":{"value":"MyFocuser"},"guidermodule":{"value":"MyGuider"}}},"startup":{"e":{"confsatstart":{"value":true},"devatstart":{"value":true},"indiatstart":{"value":true}}}}}');
INSERT INTO PROFILES VALUES('Polar','demo','{"p":{"devices":{"e":{"camera":{"value":"CCD Simulator"},"equipments":{"value":"Simulators"},"gps":{"value":"GPS Simulator"},"mount":{"value":"Telescope Simulator"}}},"optic":{"e":{"diam":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":150},"fl":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":600},"optic":{"value":"N150"},"red":{"format":"","max":10,"min":0.1,"slider":0,"step":0,"type":"float","value":1}}},"parms":{"e":{"exposure":{"format":"","max":300,"min":1e-05,"slider":2,"step":0.001,"type":"float","value":5},"gain":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100},"offset":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100}}},"server":{"e":{"host":{"value":"localhost"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}}},"startup":{"e":{"confsatstart":{"value":true},"devatstart":{"value":true},"indiatstart":{"value":true}}}}}');
INSERT INTO PROFILES VALUES('Navigator','demo','{"p":{"centeringparams":{"e":{"maxiterations":{"format":"","max":10,"min":1,"slider":2,"step":1,"type":"int","value":5},"syncafter":{"value":true},"tolerance":{"format":"%.1f","max":300,"min":5,"slider":2,"step":5,"type":"float","value":30}}},"devices":{"e":{"camera":{"value":"CCD Simulator"},"equipments":{"value":"Simulators"},"gps":{"value":"GPS Simulator"},"mount":{"value":"Telescope Simulator"}}},"optic":{"e":{"diam":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":150},"fl":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":600},"optic":{"value":"N150"},"red":{"format":"","max":10,"min":0.1,"slider":0,"step":0,"type":"float","value":1}}},"parms":{"e":{"exposure":{"format":"","max":300,"min":1e-05,"slider":2,"step":0.001,"type":"float","value":5},"gain":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100},"offset":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100}}},"server":{"e":{"host":{"value":"localhost"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}}},"slaves":{"e":{"plannermodule":{"value":"MyPlanner"}}},"startup":{"e":{"confsatstart":{"value":true},"devatstart":{"value":true},"indiatstart":{"value":true}}},"target":{"e":{"targetde":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"float","value":0},"targetname":{"value":""},"targetra":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"float","value":0}}}}}');
INSERT INTO PROFILES VALUES('Guider','demo','{"p":{"calParams":{"e":{"calsteps":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":2},"pulse":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":1000}}},"calibrationvalues":{"e":{"calMountDEC":{"format":"99.99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":55.821342216757515},"calPier":{"value":true},"calPulseE":{"format":"99.99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":1868.8577842890288},"calPulseN":{"format":"99.99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":621.4850048681824},"calPulseS":{"format":"99.99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":606.0335956263766},"calPulseW":{"format":"99.99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":1868.3412981795202},"ccdOrientation":{"format":"99.99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":30.941305107283448},"revDE":{"value":false},"revRA":{"value":false}}},"devices":{"e":{"camera":{"value":"Guide Simulator"},"equipments":{"value":"Simulators"},"guider":{"value":"Telescope Simulator"}}},"disCorrections":{"e":{"disDE+":{"value":false},"disDE-":{"value":false},"disRA+":{"value":false},"disRA-":{"value":false}}},"guideParams":{"e":{"deAgr":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":0.8},"ditherpixel":{"format":"99","max":50,"min":1,"slider":0,"step":0,"type":"int","value":5},"enablepiersidereverse":{"value":false},"pulsemax":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":2000},"pulsemin":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":20},"raAgr":{"format":"99.99","max":0,"min":0,"slider":0,"step":0,"type":"float","value":0.8},"rmsOver":{"format":"99","max":100,"min":2,"slider":0,"step":0,"type":"int","value":10}}},"optic":{"e":{"diam":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":50},"fl":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":242},"optic":{"value":"Evoguide50"},"red":{"format":"","max":10,"min":0.1,"slider":0,"step":0,"type":"float","value":1}}},"parms":{"e":{"exposure":{"format":"","max":300,"min":1e-05,"slider":2,"step":0.001,"type":"float","value":3},"gain":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100},"offset":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100}}},"revCorrections":{"e":{"revDE":{"value":false},"revRA":{"value":false}}},"server":{"e":{"host":{"value":"localhost"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}}},"startup":{"e":{"confsatstart":{"value":true},"devatstart":{"value":true},"indiatstart":{"value":true}}}}}');
INSERT INTO PROFILES VALUES('Focus','demo','{"p":{"devices":{"e":{"camera":{"value":"CCD Simulator"},"equipments":{"value":"Simulators"},"filter":{"value":"Filter Simulator"},"focuser":{"value":"Focuser Simulator"}}},"optic":{"e":{"diam":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":150},"fl":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":600},"optic":{"value":"N150"},"red":{"format":"","max":10,"min":0.1,"slider":0,"step":0,"type":"float","value":1}}},"parameters":{"e":{"aroundinitial":{"value":false},"backlash":{"format":"99999","max":0,"min":0,"slider":0,"step":0,"type":"int","value":100},"iterations":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":5},"loopIterations":{"format":"99","max":0,"min":0,"slider":0,"step":0,"type":"int","value":3},"startpos":{"format":"999999","max":0,"min":0,"slider":0,"step":0,"type":"int","value":32000},"steps":{"format":"999999","max":0,"min":0,"slider":0,"step":0,"type":"int","value":2000},"zoning":{"format":"99999","max":64,"min":1,"slider":0,"step":0,"type":"int","value":2}}},"parms":{"e":{"exposure":{"format":"","max":300,"min":1e-05,"slider":2,"step":0.001,"type":"float","value":3},"gain":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100},"offset":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100}}},"server":{"e":{"host":{"value":"localhost"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}}},"startup":{"e":{"confsatstart":{"value":true},"devatstart":{"value":true},"indiatstart":{"value":true}}}}}');
INSERT INTO PROFILES VALUES('Inspector','demo','{"p":{"devices":{"e":{"camera":{"value":"CCD Simulator"},"equipments":{"value":"Simulators"}}},"optic":{"e":{"diam":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":150},"fl":{"format":"","max":4000,"min":1,"slider":0,"step":0,"type":"float","value":600},"optic":{"value":"N150"},"red":{"format":"","max":10,"min":0.1,"slider":0,"step":0,"type":"float","value":1}}},"parameters":{"e":{"zoning":{"format":"99999","max":64,"min":1,"slider":0,"step":0,"type":"int","value":2}}},"parms":{"e":{"cornersize":{"format":"","max":2000,"min":0,"slider":2,"step":0,"type":"int","value":200},"exposure":{"format":"","max":300,"min":1e-05,"slider":2,"step":0.001,"type":"float","value":2},"gain":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100},"offset":{"format":"","max":500,"min":0,"slider":2,"step":1,"type":"int","value":100}}},"server":{"e":{"host":{"value":"localhost"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}}},"startup":{"e":{"confsatstart":{"value":true},"devatstart":{"value":true},"indiatstart":{"value":true}}}}}');
INSERT INTO PROFILES VALUES('Planner','demo','{"p":{"devices":{"e":{"equipments":{"value":"Simulators"},"gps":{"value":"GPS Simulator"}}},"planning":{"e":{"dec":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"float","value":57.032597},"object":{"value":"Megrez"},"profile":{"value":"LRGB-60s"},"progress":{"value":{"dynlabel":"Queued","value":0}},"ra":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"float","value":12.25707}},"grid":[[55.959722,"Alioth","LRGB-60s",{"dynlabel":"Queued","value":0},12.900472],[61.750833,"Dubhe","LRGB-60s",{"dynlabel":"Queued","value":0},11.062139],[57.032597,"Megrez","LRGB-60s",{"dynlabel":"Queued","value":0},12.25707]],"gridheaders":["dec","object","profile","progress","ra"]},"server":{"e":{"host":{"value":"localhost"},"port":{"format":"","max":0,"min":0,"slider":0,"step":0,"type":"int","value":7624}}},"slaves":{"e":{"navigatormodule":{"value":"MyNavigator"},"sequencemodule":{"value":"MySequencer"}}},"startup":{"e":{"confsatstart":{"value":true},"devatstart":{"value":true},"indiatstart":{"value":true}}}}}');

--DROP TABLE IF EXISTS CONFIGURATIONS;
CREATE TABLE IF NOT EXISTS CONFIGURATIONS (
        CONFIGNAME     TEXT NOT NULL,
        MODULELABEL     TEXT NOT NULL,
        MODULETYPE     TEXT NOT NULL,
        PROFILENAME    TEXT NOT NULL,
        PRIMARY KEY("CONFIGNAME","MODULELABEL")
);
DELETE FROM CONFIGURATIONS WHERE PROFILENAME='demo';
INSERT INTO CONFIGURATIONS VALUES('demo','My Sequencer','Sequencer','LRGB-60s');
--INSERT INTO CONFIGURATIONS VALUES('demo','My Indipanel','Indipanel','demo');
INSERT INTO CONFIGURATIONS VALUES('demo','My Allsky','Allsky','demo');
INSERT INTO CONFIGURATIONS VALUES('demo','My CCD inspector','Inspector','demo');
INSERT INTO CONFIGURATIONS VALUES('demo','My Focuser','Focus','demo');
INSERT INTO CONFIGURATIONS VALUES('demo','My Navigator','Navigator','demo');
INSERT INTO CONFIGURATIONS VALUES('demo','My Planner','Planner','demo');
INSERT INTO CONFIGURATIONS VALUES('demo','My Guider','Guider','demo');
INSERT INTO CONFIGURATIONS VALUES('demo','My Polar','Polar','demo');

INSERT INTO CONFIGURATIONS VALUES ('modules','My Indipanel','Indipanel','default');
INSERT INTO CONFIGURATIONS VALUES ('modules','My Allsky','Allsky','default');
INSERT INTO CONFIGURATIONS VALUES ('modules','My CCD inspector','Inspector','default');
INSERT INTO CONFIGURATIONS VALUES ('modules','My Focuser','Focus','default');
INSERT INTO CONFIGURATIONS VALUES ('modules','My Navigator','Navigator','default');

--DROP TABLE IF EXISTS INDICONFIGURATIONS;
CREATE TABLE IF NOT EXISTS INDICONFIGURATIONS (
        CONFIGNAME     TEXT NOT NULL,
        DRIVERS     TEXT NOT NULL,
        PRIMARY KEY("CONFIGNAME","DRIVERS")
);
INSERT INTO INDICONFIGURATIONS VALUES ('demo','CCD Simulator');
INSERT INTO INDICONFIGURATIONS VALUES ('demo','Focus Simulator');
INSERT INTO INDICONFIGURATIONS VALUES ('demo','Telescope Simulator');
INSERT INTO INDICONFIGURATIONS VALUES ('demo','Guide Simulator');
INSERT INTO INDICONFIGURATIONS VALUES ('demo','Filter Simulator');
INSERT INTO INDICONFIGURATIONS VALUES ('demo','GPS Simulator');

CREATE TABLE IF NOT EXISTS CATALOGS (
        CATALOG TEXT NOT NULL,
        CODE TEXT NOT NULL,
        RA REAL NOT NULL,
        NS TEXT NOT NULL,
        DEC REAL NOT NULL,
        DIAM REAL,
        MAG REAL,
        NAME TEXT,
        ALIAS TEXT,
        PRIMARY KEY("CATALOG","CODE","RA")
);
--DROP TABLE IF EXISTS OPTICS;
CREATE TABLE IF NOT EXISTS OPTICS (
        NAME     TEXT NOT NULL,
        FOCAL     REAL NOT NULL,
        DIAM     REAL NOT NULL,
        BARLOW REAL NOT NULL,
        FILTER BOOL,
        PRIMARY KEY("NAME")
);
INSERT INTO OPTICS VALUES ('SW200/800',800.0,200.0,1.0,TRUE);
INSERT INTO OPTICS VALUES ('254/1000',1000.0,254.0,1.5,FALSE);
CREATE TABLE IF NOT EXISTS USERS (
        LOGIN     TEXT NOT NULL,
        PW     TEXT NOT NULL,
        GRANT     TEXT NOT NULL, -- 0 : readOnly / 1 : read/write
        PRIMARY KEY("LOGIN")
);
INSERT INTO USERS VALUES ('GUEST','guest','0');
INSERT INTO USERS VALUES ('ADMIN','admin','1');
