# Sessie 1 - pixel manipulatie

## Opdracht 1.1: Thresholding

* Lees afbeelding `imageColor.png` en segmenteer de skin pixels
   * Gebruik hiervoor een filter uit de literatuur in het RGB domein: `(RED>95) && (GREEN>40) && (BLUE>20) && ((max(RED,max(GREEN,BLUE)) - min(RED,min(GREEN,BLUE)))>15) && (abs(RED-GREEN)>15) && (RED>GREEN) && (RED>BLUE);`
   * Visualiseer naast het masker (binaire pixels die tot huid behoren) dat je bekomt ook de resulterende pixelwaarden.
* Lees afbeelding `imageBimodal.png` in en segmenteer de tekst
   * Start met OTSU thresholding - wat gaat er mis?
   * Hoe kunnen we het originele beeld verbeteren om dit tegen te gaan?
      * Histogram equalization
      * CLAHE: Contrast Limited Adaptive Histogram Equalization
   * Merk je bepaalde problemen bij beide technieken?

## Opdracht 1.2: Erosie en dilatie

* Haal afbeelding `imageColorAdapted` door je thresholding pipeline en ga na wat er mis gaat.
* Gebruik `opening`, `closing`, `dilatie`, `erosie` om het binaire masker op te schonen en ruis te onderdrukken.
* Kan je gescheiden ledematen terug met elkaar verbinden?
* Kun je niet volledige binaire regio's terug vullen?
* Probeer in resulterende blobs regios weg te werken

Bekijk zeker `findContours()`, `convexHull` en `drawContours`!
