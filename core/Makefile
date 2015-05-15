include ../Makefile

INCLUDE = -I../include/ -I../mm/


allobj: core.o ipc.o calqueue.o message_state.o numerical.o topology.o main.o


core.o: core.c
	@echo "[CC] $@"
	@$(CC) -c core.c $(INCLUDE) $(CFLAGS) $(DEBUG) -O2

ipc.o: ipc.c
	@echo "[CC] $@"
	@$(CC) -c ipc.c $(INCLUDE) $(CFLAGS) $(DEBUG) -O2

calqueue.o: calqueue.c
	@echo "[CC] $@"
	@$(CC) -c calqueue.c $(INCLUDE) $(CFLAGS) $(DEBUG) -O2

message_state.o: message_state.c
	@echo "[CC] $@"
	@$(CC) -c message_state.c $(INCLUDE) $(CFLAGS) $(DEBUG) -O2

numerical.o: numerical.c
	@echo "[CC] $@"
	@$(CC) -c numerical.c $(INCLUDE) $(CFLAGS) $(DEBUG) -O2

topology.o: topology.c
	@echo "[CC] $@"
	@$(CC) -c topology.c $(INCLUDE) $(CFLAGS) $(DEBUG) -O2

main.o: main.c
	@echo "[CC] $@"
	@$(CC) -c main.c $(INCLUDE) $(CFLAGS) $(DEBUG) -O2
