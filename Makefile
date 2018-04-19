# Makefile for parent and child signal communication programs.
# version: 1.0  (parent adds numbers, so no need to differentiate)

RM ?= rm -f

targets = parent child
toclean += $(targets)

all: $(targets)
clean:
	$(RM) $(toclean)

parent_objs = parent.o
toclean += $(parent_objs)
child_objs = child.o
toclean += $(child_objs)

parent: $(parent_objs)
	$(CC) $(LDFLAGS) -o $@ $(parent_objs)

child: $(child_objs)
	$(CC) $(LDFLAGS) -o $@ $(child_objs)
