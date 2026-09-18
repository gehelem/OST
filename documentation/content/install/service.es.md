---
title: Gestionar el servicio ostserver
weight: 30
description: "Iniciar, detener, personalizar las opciones de arranque del servicio ostserver"
---

`ostserver` se ejecuta en segundo plano como **servicio systemd**. Systemd es el gestor de servicios de Linux: inicia automáticamente `ostserver` al arrancar la máquina, lo reinicia si falla, y permite controlarlo con algunos comandos simples.

---

## Iniciar, detener, reiniciar

```bash
# Iniciar el servicio
sudo systemctl start ostserver

# Detener el servicio
sudo systemctl stop ostserver

# Reiniciar el servicio (útil tras modificar las opciones)
sudo systemctl restart ostserver

# Ver el estado del servicio
sudo systemctl status ostserver
```

---

## Activar / desactivar el arranque automático

Por defecto, el servicio está **activado**: se inicia automáticamente en cada arranque de la máquina.

```bash
# Desactivar el arranque automático
sudo systemctl disable ostserver

# Reactivar el arranque automático
sudo systemctl enable ostserver
```

---

## Consultar los logs

```bash
# Mostrar los últimos logs
sudo journalctl -u ostserver

# Seguir los logs en tiempo real
sudo journalctl -u ostserver -f

# Limitar a las últimas 100 líneas
sudo journalctl -u ostserver -n 100
```

---

## Personalizar las opciones de arranque

Durante la instalación, se crea un archivo de configuración en su directorio de usuario:

```
~/.ost/ostserver.parms
```

Este archivo contiene **un argumento por línea**. Las líneas que empiezan por `#` son comentarios y se ignoran. Los espacios en los valores son compatibles de forma nativa — no se necesitan comillas.

Para editarlo:

```bash
nano ~/.ost/ostserver.parms
```

Ejemplo de contenido:

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
--banner=Mi observatorio sin cabeza
```

Tras cualquier modificación, reinicie el servicio para que se aplique:

```bash
sudo systemctl restart ostserver
```

{{% notice style="info" title="Archivo preservado durante las actualizaciones" %}}
El archivo `ostserver.parms` **nunca se sobrescribe** durante una actualización del paquete. Sus personalizaciones se conservan.
{{% /notice %}}

---

## Referencia de las opciones disponibles

| Opción | Por defecto | Descripción |
|---|---|---|
| `--webroot=<ruta>` | `/var/lib/osterix/media` | Carpeta de medios servida por nginx |
| `--indiserver=Y\|N` | `N` | Iniciar el servidor INDI embebido |
| `--loglevel=<0-4>` | `1` | Verbosidad de los logs: 0=debug, 1=info, 2=warning, 3=error, 4=critical |
| `--logfile=<ruta>` | *(con marca de tiempo en `~/.ost/`)* | Ruta del archivo de log |
| `--grant=<0\|1\|2>` | — | Seguridad: 0=desactivada, 1=solo lectura, 2=permisos de usuario |
| `--lng=<fr\|en\|...>` | `fr` | Idioma de la interfaz (fr, en, de, it, es) |
| `--banner=<texto>` | `Observatoire Sans tête` | Banner mostrado en el frontend |
| `--configuration=<nombre>` | `default` | Nombre de la configuración a cargar al arrancar |
| `--libpath=<ruta>` | *(ruta del sistema)* | Directorio de los módulos `.so` |
| `--dbpath=<ruta>` | *(directorio de trabajo)* | Ubicación de la base de datos |
| `--systemwatchinterval=<s>` | `10` | Intervalo de vigilancia del sistema en segundos (0 para desactivar) |
| `--minfree=<0-99>` | `10` | Espacio en disco libre mínimo en % antes de omitir la escritura de una imagen |
| `--ssl=Y\|N` | `N` | Activar SSL para el WebSocket |
| `--sslcert=<ruta>` | `/etc/ost/server.crt` | Certificado SSL |
| `--sslkey=<ruta>` | `/etc/ost/server.key` | Clave privada SSL |
| `--setadminpassword=<contraseña>` | — | Definir la contraseña de administrador (eliminar tras su uso) |
