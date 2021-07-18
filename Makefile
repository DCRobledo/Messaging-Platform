CC=gcc
CFLAGS=
JC=javac

SRC=server/src
SRCSRPC=server_rpc/src

OBJ=server/obj
OBJSRPC=server_rpc/obj

JSRC=client/src
RPCSRC=RPC
WS=WebService/textConversor
RPCGENFLAGS = -N -M
JSRC2=$(WS)/src

serverFiles=$(wildcard $(SRC)/*.c)
OBJS=$(patsubst $(SRC)%.c,$(OBJ)%.o, $(serverFiles))

serverRPCFiles=$(wildcard $(SRCSRPC)/*.c)
OBJSSRPC=$(patsubst $(SRCSRPC)%.c,$(OBJSRPC)%.o, $(serverRPCFiles))

JOBJS=$(wildcard $(JSRC)/*.java)
JOBJS2=$(wildcard $(JSRC2)/*.java)

LDFLAGS = -L$(INSTALL_PATH)/lib/
LDLIBS = -lrt -pthread

OUT=server/bin
RPCOUT=server_rpc/bin
JOUT=client/bin
JOUT2=$(WS)/bin
TMPDIR=$(OUT) $(OBJ) $(OBJSRPC) $(RPCOUT) $(JOUT) $(JOUT2)

all: configure rpcgen server server_rpc client webservice
	@echo "Compilation successfully"

configure:
	@echo "Create the tmp dirs"
	$(foreach dir, $(TMPDIR), [ -d $(dir) ] || mkdir -p $(dir); )
	$(foreach file, $(JOBJS), touch $(file))
	#[ -d $(OUT) ] || mkdir $(OUT)
	#[ -d $(JOUT) ] || mkdir $(JOUT)
	#[ -d $(JOUT2) ] || mkdir $(JOUT2)
	#[ -d $(RPCOUT) ] || mkdir $(RPCOUT)

server: server/obj/register_archive_xdr.o server/obj/register_archive_clnt.o $(OBJS)
	@echo "compiling the server..."
	$(CC) $(LDLIBS) $^ -o $(OUT)/$@

server_rpc: server/obj/linked_list.o server/obj/register_archive_xdr.o server/obj/register_archive_svc.o $(OBJSSRPC)
	@echo "compiling the RPC server..."
	$(CC) $(LDLIBS) $^ -o $(RPCOUT)/$@

client: $(JOBJS)
	@echo "compoiling the client..."
	$(JC) -classpath client/src/:WebService/ $^ -d $(JOUT)

webservice: $(JOBJS2)
	@echo "compiling the webService..."
	$(JC) -classpath client/src/:WebService/ $^ -d $(JOUT2)

rpcgen:
	@echo "Geneate the rpc..."
	cd $(RPCSRC) && rpcgen $(RPCGENFLAGS) register_archive.x

clean:
	@echo "Cleaning the project"
	$(foreach dir, $(TMPDIR), echo "Remove $(dir)"; rm -r $(dir) 2>>/dev/null || true; )

debug: $(JOBJS)
	echo "$(JC) -classpath client/src/:WebService/ $^ -d $(JOUT)"


$(OBJ)%.o: $(SRC)%.c
	$(CC) $(CFLAGS) $(LDGLAGS) $^ $(LDLIBS) -c -o $@

$(OBJ)%.o: $(RPCSRC)%.c
	$(CC) $(CFLAGS) $(LDGLAGS) $^ $(LDLIBS) -c -o $@

$(OBJSRPC)%.o: $(SRCSRPC)%.c
	$(CC) $(CFLAGS) $(LDGLAGS) $^ $(LDLIBS) -c -o $@

