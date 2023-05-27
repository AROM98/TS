Para compilar:

gcc -Wall hello_rsyslog.c `pkg-config fuse3 --cflags --libs` -o hello_logger

Para observar os logs:

tail -f /var/log/syslog | grep "FUSE-LOGS"
