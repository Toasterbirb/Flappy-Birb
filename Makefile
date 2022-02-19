cc=g++
files=./src/*
outputDir=./build
binary=game
resources=./res
flags=-lSDL2 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer

game:
	mkdir -p ${outputDir}
	cp -r ${resources} ${outputDir}/
	${cc} -I./include -lbirb2d -g ${files} ${flags}  -o ${outputDir}/${binary}_debug

clean:
	rm -rf ${outputDir}
