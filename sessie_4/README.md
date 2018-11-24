# Sessie 3 - keypoint detection and matching

Met template matching gaan we op zoek naar een exact object. Wanneer er echter een grote vervorming van het gekende object kan optreden, dan lijkt template matching al snel te falen. In dit labo voeren we een analyse uit van een eyetracker opname waarbij we op zoek gaan naar een vooraf bepaald object (kinderbueono and fitness cornflakes).

* **Opdracht 1**: Bepaal op de inputbeelden en op de te zoeken templates keypoints, gebaseerd op de volgende keypoint types: ORB, BRISK en AKAZE.
* **Opdracht 2**: Kies 1 van bovenstaande keypoint detectoren, bepaal descriptoren van de gegenereerde keypoints en doe een brute force matching tussen beide sets.
* **Opdracht 3**: Pas RANSAC toe om enkel de keypoint matches over te houden die op iets slaan. Gebruik de resulterende matches om een homography uit te rekenen en het object in uw doelbeeld te vinden.

# Usage

Enter one of the tasks folders using `cd opdracht_1` or `cd opdracht_2` or `cd opdracht_3`.
In this directory:
- `cmake CMakeLists.txt`
- `make`
- `./sessie_4-1 --input=fitness_image.jpg --object=fitness_object.jpg` or `./sessie_4-2 --input=fitness_image.jpg --object=fitness_object.jpg` or `./sessie_4-3 --input=fitness_image.jpg --object=fitness_object.jpg`

:bulb: You can also use `--input=kinderbueno_image.jpg --object=kinderbueno_object.jpg` when you run these projects.
