
CC = gcc

INCLUDE = -I include/ 

FLAGS = -Wall -mrtm -pthread -lm

CFLAGS = $(FLAGS)

#Insert -g to enable debug mode
DEBUG = 

TARGET = test

ifndef NO_DYMELOR
ENGINE_RULE = compile_core compile_mm linking_application_to_mm output
else
CFLAGS = $(FLAGS) -DNO_DYMELOR
ENGINE_RULE = compile_core compile_mm output
endif

ifdef THROTTLING
CFLAGS += -DTHROTTLING
endif

MM_SOURCES=mm/allocator.c\
		mm/dymelor.c\
		mm/recoverable.c


PCS_SOURCES=model/pcs/application.c\
		model/pcs/functions_app.c


PCS_PREALLOC_SOURCES=model/pcs-prealloc/application.c\
		    model/pcs-prealloc/functions_app.c\
		    model/pcs-prealloc/topology.c


PHOLD_SOURCES=model/phold/application.c


TCAR_SOURCES=model/tcar/application.c


TRAFFIC_SOURCES=model/traffic/application.c\
		    model/traffic/functions.c\
		    model/traffic/init.c\
		    model/traffic/normal_cdf.c


CORE_SOURCES=core/message_state.c\
		core/core.c\
		core/calqueue.c\
		core/topology.c\
		core/ipc.c\
		core/pool_allocator.c\
		core/main.c\
		core/numerical.c


CORE_OBJ=$(CORE_SOURCES:.c=.o)
MM_OBJ=$(MM_SOURCES:.c=.o)
PCS_OBJ=$(PCS_SOURCES:.c=.o)
PCS_PREALLOC_OBJ=$(PCS_PREALLOC_SOURCES:.c=.o)
TRAFFIC_OBJ=$(TRAFFIC_SOURCES:.c=.o)
TCAR_OBJ=$(TCAR_SOURCES:.c=.o)
PHOLD_OBJ=$(PHOLD_SOURCES:.c=.o)


all: pcs

pcs: compile_pcs $(ENGINE_RULE)

pcs-prealloc: compile_pcs-prealloc $(ENGINE_RULE)

phold: compile_phold $(ENGINE_RULE)

tcar: compile_tcar $(ENGINE_RULE)

traffic: compile_traffic $(ENGINE_RULE)


compile_core:
	make -C core/ allobj


compile_mm: $(MM_OBJ)
	@ld $(DEBUG) -r $(MM_OBJ) -o mm/__mm.o

$(MM_OBJ): %.o: %.c
	@echo "[CC] $@"
	@$(CC) -o $@ -c $< $(CFLAGS) $(DEBUG) $(INCLUDE)


# -------------------------------- Application rules --------------------------------

compile_pcs: $(PCS_OBJ)
	@ld $(DEBUG) -r $(PCS_OBJ) -o model/__application.o

$(PCS_OBJ): %.o: %.c
	@echo "[CC] $@"
	@$(CC) -o $@ -c $< $(CFLAGS) $(DEBUG) $(INCLUDE) -O1

compile_pcs-prealloc: $(PCS_PREALLOC_OBJ)
	@ld $(DEBUG) -r $(PCS_PREALLOC_OBJ) -o model/__application.o

$(PCS_PREALLOC_OBJ): %.o: %.c
	@echo "[CC] $@"
	@$(CC) -o $@ -c $< $(CFLAGS) $(DEBUG) $(INCLUDE)

compile_phold: $(PHOLD_OBJ)
	@ld $(DEBUG) -r $(PHOLD_OBJ) -o model/__application.o

$(PHOLD_OBJ): %.o: %.c
	@echo "[CC] $@"
	@$(CC) -o $@ -c $< $(CFLAGS) $(DEBUG) $(INCLUDE) -O1

compile_tcar: $(TCAR_OBJ)
	@ld $(DEBUG) -r $(TCAR_OBJ) -o model/__application.o

$(TCAR_OBJ): %.o: %.c
	@echo "[CC] $@"
	@$(CC) -o $@ -c $< $(CFLAGS) $(DEBUG) $(INCLUDE)

compile_traffic: $(TRAFFIC_OBJ)
	@ld $(DEBUG) -r $(TRAFFIC_OBJ) -o model/__application.o

$(TRAFFIC_OBJ): %.o: %.c
	@echo "[CC] $@"
	@$(CC) -o $@ -c $< $(CFLAGS) $(DEBUG) $(INCLUDE)


linking_application_to_mm:
	ld $(DEBUG) -r --wrap malloc --wrap free --wrap realloc --wrap calloc -o model/application-mm.o model/__application.o --whole-archive mm/__mm.o


output:
ifndef NO_DYMELOR
	$(CC) -o $(TARGET) -Wall model/application-mm.o $(CORE_OBJ) $(CFLAGS) $(DEBUG)
else
	$(CC) -o $(TARGET) -Wall model/__application.o $(CORE_OBJ) $(CFLAGS) $(DEBUG)
endif


clean:
	@find . -name "*.o" -exec rm {} \;
	@rm -f $(TARGET)
