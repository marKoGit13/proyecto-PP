all: build_SupportFuncs build_Events build_EventBus build_Component build_Entity build_Systems build_World build_Game build_main link

build_SupportFuncs:
	g++ -c ./src/SupportFuncs.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include -Iexternal/nlohmann_json/include  \
		-o ./bin/SupportFuncs.o
build_Events:
	g++ -c ./src/Event.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include \
		-o ./bin/Event.o
build_EventBus:
	g++ -c ./src/EventBus.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include \
		-o ./bin/EventBus.o
build_Component:
	g++ -c ./src/Component.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include \
		-o ./bin/Component.o

build_Entity:
	g++ -c ./src/Entity.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include \
		-o ./bin/Entity.o

build_Systems:
	g++ -c ./src/ISystem.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include -Iexternal/nlohmann_json/include  \
		-o ./bin/ISystem.o

build_World:
	g++ -c ./src/World.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include -Iexternal/nlohmann_json/include  \
		-o ./bin/World.o

build_Game:
	g++ -c ./src/Game.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include -Iexternal/nlohmann_json/include  \
		-o ./bin/Game.o
build_main:
	g++ -c ./src/main.cpp \
		-Iexternal/spdlog/include -Iexternal/glm \
		-Iexternal/SDL3/include -Iexternal/SDL3_image/include -Iexternal/nlohmann_json/include \
		-o ./bin/main.o

link: ./bin/SupportFuncs.o ./bin/Event.o ./bin/EventBus.o ./bin/Component.o ./bin/Entity.o ./bin/ISystem.o ./bin/World.o ./bin/Game.o ./bin/main.o
	g++ ./bin/SupportFuncs.o ./bin/Event.o ./bin/EventBus.o ./bin/Component.o ./bin/Entity.o ./bin/ISystem.o ./bin/World.o ./bin/Game.o ./bin/main.o\
		-Lexternal/spdlog -lspdlog \
		-Lexternal/glm-master -lglm \
		-Lexternal/SDL3/lib -lSDL3 \
		-Lexternal/SDL3_image/lib -lSDL3_image \
		-lwinmm -limm32 -lversion -lole32 -loleaut32 \
		-lsetupapi -lshell32 -luser32 -lgdi32 -luuid \
		-o ./bin/game.exe

run:
	./bin/game.exe

clean:
	del /Q ./bin/*.o
	del /Q ./bin/*.exe