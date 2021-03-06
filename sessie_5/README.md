# Sessie 5 - Machine Learning

Een aardbei plukker wil graag mee gaan met de tijd en wil zijn manuele arbeid
reduceren. Hiervoor heeft hij aan ons gevraag om een automatische aarbei-pluk
machine te bouwen, die in staat is rijpe aardbeien te onderscheiden van onrijpe
aardbeien. In dit labo gaan jullie proberen om met behulp van machine learning
rijpe aarbeien te detecteren.

* **Opdracht 1**: Maak een interface waarop je op basis van *mouse callbacks*
  pixels kan aanklikken. Bij links klikken, sla je een extra punt op. Rechts
  klikken verwijder je het laatst toegevoegde punt. En waarbij de middelste knop
  een lijst toont met alle opgeslagen punten.

* **Opdracht 2**: Op basis van de geselecteerd punten bepaalje een descriptor,
  in dit geval de HSV representatie van de pixel.

* **Opdracht 3**: Train met deze data een K-Nearest-Neighbor classifier, een
  Normal Bayes Classifier en een Support Vector Machine

* **Opdracht 4**: Classificeer de pixels van de afbeelding met deze technieken
  en visualiseer met een masker de resterende pixels.

* **EXTRA**: Hoe kan je de segmentatie verbeteren? Denk na over het feit dat een
  rijpe aarbei meer rood en minder groen componenten geven.

# Usage

Enter one of the tasks folders using `cd opdracht_1` or `cd opdracht_2` or `cd opdracht_3` or `cd opdracht_4` or `cd opdracht_extra`.
In this directory:
- `cmake CMakeLists.txt`
- `make`
- `./sessie_5-1 --strawberry=strawberry1.tif` or `./sessie_5-2 --strawberry=strawberry1.tif` or `./sessie_5-3 --strawberry=strawberry1.tif` or `./sessie_5-4 --strawberry=strawberry1.tif` or `./sessie_5-extra --strawberry=strawberry1.tif`

:bulb: You can also use `--strawberry=strawberry2.tif` when you run these projects.
