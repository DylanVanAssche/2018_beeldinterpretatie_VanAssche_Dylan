# Sessie 3 - template based matching

Kleur segmentatie en connected components brengen je maar tot een beperkte oplossing bij heel wat problemen. Daarom gaan we deze week een stap verder. Op een transportband in een industriele setup komen zowel metalen plaatjes als metalen ringen voorbij (zie input foto). De metalen plaatjes moeten gedetecteerd worden en gelabeld als in de output foto, zodanig dat een autonome robot deze van de band kan plukken. Hiervoor zullen we gebruik maken van template matching.

* Opdracht 1: Gebruik template matching om een object te vinden in een inputbeeld
* Opdracht 2: Pas de template matching aan om lokaal naar maxima te zoeken, zodanig dat je alle matches vind
* Extra: Pas de template matching aan geroteerde objecten te vinden (roteren van beeld, rotatedRect, warpPerspective op hoekpunten)

Tip voor extra opdracht: https://github.com/OpenCVBlueprints/OpenCVBlueprints/blob/master/chapter_5/source_code/rotation_invariant_detection/rotation_invariant_object_detection.cpp

# Usage

Enter one of the tasks folders using `cd opdracht_1` or `cd opdracht_2` or `cd opdracht_extra`.
In this directory:
- `cmake CMakeLists.txt`
- `make`
- `./sessie_3-1 --input=input.jpg --template=template.jpg` or `./sessie_3-2 --input=recht.jpg --template=template.jpg` or `./sessie3-extra --input=rot.jpg --template=template.jpg`

