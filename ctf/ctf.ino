#include <LiquidCrystal.h>
#include "character.h"


//--------------------------------Definitions----------------------------------

// Nombre de minutes max affichées dans la selection du temps
// (Ne pas allez au dessus de 99, l'affichage n'est pas géré pour
// les nombre à trois chiffre)
#define MAX_MINUTES 15

// Decompte pour depart de la partie, 0 si pas de decompte
// (Ne pas allez au dessus de 99, l'affichage n'est pas géré pour
// les nombre à trois chiffre)
#define DECOMPTE 3

// Pins de l'écran LCD (4 pin de données)
#define LCD_RS 7
#define LCD_EN 6
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

// Pins des buttons lumineux
#define BLUE_LED 10
#define BLUE_BUTTON 13
#define RED_LED 9
#define RED_BUTTON 12
// On utilise pas le 11 car l'utisation du buzzer (fonction tone)
// interfère avec le PGM sur ce pin
// On ne peut pas faire PGM et analogWrite en même temps

// Pin du buzzer
#define BUZZER 11
// On peut donc utiliser le 11 pour le buzzer lui même

//Sons du Buzzer
#define BIP_SOUND_FREQUENCY 220 // Bip de selection (220)
#define BIP_SOUND_DURATION 9 // (9)
#define START_SOUND_FREQUENCY 1120 // Son de depart et de decompte (1120)
#define START_SOUND_DURATION_SHORT 40 // (40)
#define START_SOUND_DURATION_LONG 800 // (800)
#define CAPTURE_SOUND_FREQUENCY 880 // Son lors de la capture (880)
#define CAPTURE_SOUND_DURATION_ONE_TIME 120 // (120)
#define CAPTURE_SOUND_DURATION_TWO_TIME 40 // (40)
#define CAPTURE_SOUND_DURATION_THREE_TIME 40 // (60)
#define END_SOUND_FREQUENCY 1120 // Son de fin de partie (1120)

// Reset Timeout (Nombre d'heure de partie avant que la carte se reset
// car elle à sûrement été oubliée) (ne doit pas dépasser 255 (unsigned char))
#define RESET_TIMEOUT 24

// Delai anti-rebont boutons
#define ANTI_REBONT 75

// Creation de l'objet lcd
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Variables communes de temps
unsigned short tempsCapture;
unsigned short captureBleu;
unsigned short captureRouge;
unsigned long tempsDebut;
unsigned long tempsCourant;



//----------------------------------reset--------------------------------------

void (* resetArduino) (void) = 0;
// L'arduino a une fonction de reset à l'adresse 0
// On defini içi une fonction permettant de l'appeler


//-----------------------------------Setup-------------------------------------

void setup() {
	// Initialisation des pins
	initPin();

	// Envoi des charactères spéciaux à l'écran
	loadCharacter(lcd);

	// Accueil
	accueil();

	// Selection du temps de jeu
	tempsCapture = selectionTemps();

	// Start
	start();
	tempsDebut = millis();
	captureBleu = tempsCapture;
	captureRouge = tempsCapture;

	//Mise en place de l'ecran de chronomètre
	ecranChrono();
}


//------------------------------------Loop-------------------------------------

void loop() {
	if (digitalRead(RED_BUTTON)) {
		captureRouge = capture(RED_BUTTON, RED_LED, BLUE_LED);
		ecranChrono();
	}
	else if (digitalRead(BLUE_BUTTON)) {
		captureBleu = capture(BLUE_BUTTON, BLUE_LED, RED_LED);
		ecranChrono();
	}
	else {
		tempsCourant = millis() - tempsDebut;
		updateTime();
	}
	delay(10);
}


//--------------------------------waitButton-----------------------------------

void waitButton(unsigned char pin) {
	//Fonction qui sert a attendre qu'on relache le bouton corespondant

	tone(BUZZER, BIP_SOUND_FREQUENCY, BIP_SOUND_DURATION); // Bip
	while(digitalRead(pin)); // Attend qu'on relache
	delay(ANTI_REBONT); // Pour eviter des problèmes
}

void waitButtons() {
	// Fonction qui sert a attendre que l'on appuie et relache l'un des deux
	// boutons

	// Attente d'un appui sur n'importe lequel
	while(!digitalRead(RED_BUTTON) && !digitalRead(BLUE_BUTTON));

		// Puis attente que le relache le même
		if (digitalRead(RED_BUTTON)) waitButton(RED_BUTTON);
		else waitButton(BLUE_BUTTON);
}


//----------------------------affichageDizaines--------------------------------

void affichageDizaines(unsigned char nombre, unsigned char col, unsigned char row, char complete = '0') {
	// Fonction pour afficher le nombre 'nombre' à l'emplacement (col, row)
	// Elle affichera sur deux caractères que ce soit une unité ou une
	// dizaine. Dans le cas d'une unité, elle complete par 'complete'
	// /!\ Attention : ne verifie pas si nombre < 100

	lcd.setCursor(col,row);
	if (nombre < 10) lcd.print(complete);
	lcd.print(nombre, DEC);
}

//----------------------------------initPin------------------------------------

void initPin() {
	// Fonction qui gère toute les initialisations

	// Initialisation de l'écran LCD
	lcd.begin(16, 2);

	// pinMode boutons
	pinMode(BLUE_LED, OUTPUT);
	pinMode(RED_LED, OUTPUT);
	pinMode(BLUE_BUTTON, INPUT);
	pinMode(RED_BUTTON, INPUT);
}


//---------------------------------accueil-------------------------------------

void accueil() {
	// Ecran de demmarage, attend que l'on appuie sur une bouton

	// Affichage
	lcd.clear();
	lcd.print("Capture the Flag");
	lcd.setCursor(0, 1);
	lcd.print("-   Demarrer   -");

	// Allumage des boutons
	digitalWrite(BLUE_LED, HIGH);
	digitalWrite(RED_LED, HIGH);

	// Attente de l'appui sur un bouton
	waitButtons();
}


//------------------------------selectionTemps---------------------------------

unsigned short selectionTemps() {
	// Fonction qui gere la selection du temps necessaire à la capture
	// du drapeau et le retourne

	// Variables minutes et secondes
	unsigned char min;
	unsigned char sec;

	// Est-ce que le temps est correct ?
	boolean confirme;

	do {

		// Affichage des informations
		lcd.clear();
		lcd.print("Temps de capture");
		lcd.setCursor(0, 1);
		lcd.print("Minutes :   0min");
		min = 0;

		// Selection des minutes (On valide avec le bouton rouge)
		while(!digitalRead(RED_BUTTON)) {

			// On incremente d'une minute avec le bouton bleu
			if (digitalRead(BLUE_BUTTON)) {

				// Incrementation (Si on depasse le max on reprend à zéro)
				min++;
				if (min > MAX_MINUTES) min = 0;

				// Affichage de la minute
				affichageDizaines(min, 11, 1, ' ');

				waitButton(BLUE_BUTTON); // Pour attendre qu'on relache le bouton
			}
		}
		waitButton(RED_BUTTON); // Pour attendre qu'on relache le bouton


		// Affichage des informations
		lcd.setCursor(0, 1);
		lcd.print("Secondes :    0s");
		sec = 0;

		// Selection des secondes (On valide avec le bouton rouge)
		while(!digitalRead(RED_BUTTON)) {

			// On incremente de 5 secondes avec le bouton bleu
			if (digitalRead(BLUE_BUTTON)) {

				// Incrementation (Si on depasse 60sec on reprend à zéro)
				sec += 5;
				if (sec >= 60) sec = 0;

				// Affichage des secondes
				affichageDizaines(sec, 13, 1, ' ');

				waitButton(BLUE_BUTTON); // Pour attendre qu'on relache le bouton
			}
		}
		waitButton(RED_BUTTON); // Pour attendre qu'on relache le bouton


	// Affichage du temps recu et confirmation
	lcd.clear();
	lcd.print("Temps :      :");
	affichageDizaines(min, 11, 0);
	affichageDizaines(sec, 14, 0);
	lcd.setCursor(0,1);
	lcd.print("Confirmer :  Oui");
	confirme = true;

	// Validation finale (On appuie sur le bouton rouge pour continuer)
	while(!digitalRead(RED_BUTTON)) {

		// On selectionne Oui/Non avec le bouton bleu
		if (digitalRead(BLUE_BUTTON)) {

			// On change Oui/Non
			confirme = !confirme;

			// Affiche Oui ou Non
			lcd.setCursor(13,1);
			if (confirme) lcd.print("Oui");
			else lcd.print("Non");

			waitButton(BLUE_BUTTON); // Pour attendre qu'on relache le bouton
		}
	}
	waitButton(RED_BUTTON); // Pour attendre qu'on relache le bouton

} while (!confirme);   // Tant que l'on a pas mis Oui, on recommencera

	// Retourne le temps en seconde
	return ((min*60) + sec);
}


//-------------------------------waitForStart----------------------------------

void start() {
	// Ecran pour le demarrage de la partie,
	// attend que l'on appuie sur une bouton puis affiche un decompte.

	// Affichage
	lcd.clear();
	lcd.print("  Commencer la");
	lcd.setCursor(0, 1);
	lcd.print("   partie  ->");

	// Attente de l'appui sur un bouton
	waitButtons();

	if (DECOMPTE) {

		// Affichage du texte
		lcd.clear();
		lcd.print("  Depart dans");
		lcd.setCursor(0, 1);
		lcd.print("       ...");

		// Variable pour animation des LEDs
		boolean ledState = LOW;

		// Decompte
		for (size_t i=DECOMPTE; i>0; i--) {

			// Animation des LEDs
			ledState = !ledState;
			digitalWrite(BLUE_LED, ledState);
			digitalWrite(RED_LED, !ledState);

			// Affichage des chiffres
			affichageDizaines(i, 5, 1, ' ');

			// Son
			tone(BUZZER, START_SOUND_FREQUENCY, START_SOUND_DURATION_SHORT);

			// On attend une seconde
			delay(1000);
		}
	}

	// Affichage du texte
	lcd.clear();
	lcd.print(" C'est parti !");

	// Son de Depart
	tone(BUZZER, START_SOUND_FREQUENCY, START_SOUND_DURATION_LONG);

	//Animation des LEDs et delai
	digitalWrite(BLUE_LED, LOW);
	digitalWrite(RED_LED, LOW);
	delay(80);
	digitalWrite(BLUE_LED, HIGH);
	digitalWrite(RED_LED, HIGH);
	delay(80);
	digitalWrite(BLUE_LED, LOW);
	digitalWrite(RED_LED, LOW);
	delay(80);
	digitalWrite(BLUE_LED, HIGH);
	digitalWrite(RED_LED, HIGH);
	delay(80);
	digitalWrite(BLUE_LED, LOW);
	digitalWrite(RED_LED, LOW);
	delay(80);
	digitalWrite(BLUE_LED, HIGH);
	digitalWrite(RED_LED, HIGH);
	delay(80);
	digitalWrite(BLUE_LED, LOW);
	digitalWrite(RED_LED, LOW);
}

//-------------------------------updateTime------------------------------------

void updateTime() {
	// Fonction qui met à jour l'affichage du temps de chrono utilise tempsCourant

	// Mise en place des variables de temps
	unsigned char sec = (tempsCourant / 1000) % 60;
	unsigned char min = (tempsCourant / 60000) % 60;
	unsigned char heure = tempsCourant / 3600000;

	// Si la partie tourne depuis plus de RESET_TIMEOUT,
	// c'est qu'elle a surement été oubliée
	if (heure > RESET_TIMEOUT) resetArduino;

	// Affichage
	affichageDizaines(heure, 1, 1); // Heure
	affichageDizaines(min, 5, 1); // Minutes
	affichageDizaines(sec, 11, 1); // Secondes
}


//-------------------------------ecranChrono-----------------------------------

void ecranChrono() {

	// Affichage du texte
	lcd.clear();
	lcd.print("Partie en cours");
	lcd.setCursor(0, 1);
	lcd.print("   h   min   s");

	// Update du temps courant
	tempsCourant = millis() - tempsDebut;

	// Affichage du temps
	updateTime();
}

//----------------------------------capture------------------------------------

unsigned short capture(unsigned char pinBouton, unsigned char pinLed, unsigned char pinAutreLed) {
	// Fonction appelée lorsque qu'une equipe appuye sur son bouton.
	// Elle gère l'affichage et retourne à la fin le nouveau temps de l'equipe.

	// On allume la led de l'equipe
	digitalWrite(pinLed, HIGH);

	// Variable de temps
	unsigned short tempsCouleurInitial; // Le temps restant de l'equipe au moment de l'appui sur le bouton
	unsigned long tempsDebutCapture = millis(); // Le temps qu'il est pour l'Arduino au moment de l'appui sur le bouton
	unsigned long intervalleTemps; // Le temps d'appui sur le bouton
	unsigned short tempsCouleur; // Le temps restant mis à jour de la couleur
	unsigned char sec; // Secondes du temps restant
	unsigned char min; // Minutes du temps restant

	// Variables pour l'animation des led et du sons
	boolean active = false; // Pour savoir si on a deja activer/desactiver la LED et le buzzer
	boolean finProche = false; // Pour savoir si la fin est proche (on aura alors des clignotement et des bip plus rapides) (30 sec)
	boolean finTresProche = false; // Pour savoir si la fin est très très proche (on aura alors des clignotement et des bip encore plus rapides) (10 sec)
	unsigned short milliseconde; // Millisecondes
	boolean tranche1, tranche2, tranche3; // Tests pour savoir si on est dans une tranche de temps ou non

	lcd.clear();
	if (pinBouton == RED_BUTTON) {
		lcd.print(" Rouge :    :");
		tempsCouleurInitial = captureRouge;
	}
	else if (pinBouton == BLUE_BUTTON) {
		lcd.print(" Bleu :     :");
		tempsCouleurInitial = captureBleu;
	}

	do {

		// Temps passé à appuyer
		intervalleTemps = millis() - tempsDebutCapture;
		tempsCouleur = (tempsCouleurInitial - (intervalleTemps/1000));
		sec = tempsCouleur % 60;
		min = tempsCouleur / 60;

		if (tempsCouleur <= 0) finPartie(pinBouton, pinLed, pinAutreLed);

		// Affichage
		affichageDizaines(min, 10, 0);
		affichageDizaines(sec, 13, 0);
		affichageBarre(tempsCouleur);

		// On calcule les millisecondes pour l'affichage
		milliseconde = intervalleTemps%1000;

		// Animation de LEDs et sons
		if (!finTresProche) { // Si le temps est superieur à 10 sec
			if (!finProche) { // Si le temps restant est superieur à 30 sec

				// Si on est à 30 sec, on active le mode finProche
				if (min <= 0 && sec <= 30) finProche = true;

				// On regarde dans quelle partie de la seconde on est
				tranche1 = milliseconde < 500;

				// Lorsqu'on est dans la première moitié d'une seconde, la LED
				// va s'allumer et le buzzer emmetre un son. La LED s'eteindra
				// dans la deuxième partie de la seconde.
				if (!active && tranche1) {
					digitalWrite(pinAutreLed, HIGH); // On allume l'autre LED
					tone(BUZZER, CAPTURE_SOUND_FREQUENCY, CAPTURE_SOUND_DURATION_ONE_TIME); // On fait un son
					active = true;
				}
				else if (active && !tranche1) {
					digitalWrite(pinAutreLed, LOW); // On eteind l'autre LED
					active = false;
				}
			}
			else { // Fin Proche

				// Si on est à 10 sec, on active le mode finProche
				if (sec <= 10) finTresProche = true;

				// On regarde dans quelle partie de la seconde on est
				tranche1 = milliseconde < 150;
				tranche2 = milliseconde < 500 && milliseconde > 350;

				// Meme principe que ci dessus mais le nombre de bip et le timing change
				if (!active && (tranche1 || tranche2)) { // Lors des secondes paires
					digitalWrite(pinAutreLed, HIGH); // On allume l'autre LED
					tone(BUZZER, CAPTURE_SOUND_FREQUENCY, CAPTURE_SOUND_DURATION_TWO_TIME); // On fait un son
					active = true;
				}
				else if (active && (!tranche1 || !tranche2)) {
					digitalWrite(pinAutreLed, LOW); // On eteind l'autre LED
					active = false;
				}
			}
		}
		else { // FinTresProche
			// On regarde dans quelle partie de la seconde on est
			tranche1 = milliseconde < 150;
			tranche2 = milliseconde < 450 && milliseconde > 300;
			tranche3 = milliseconde > 600 && milliseconde < 750;

			// Meme principe que ci dessus mais le nombre de bip et le timing change
			if (!active && (tranche1 || tranche2 || tranche3)) { // Lors des secondes paires
				digitalWrite(pinAutreLed, HIGH); // On allume l'autre LED
				tone(BUZZER, CAPTURE_SOUND_FREQUENCY, CAPTURE_SOUND_DURATION_THREE_TIME); // On fait un son
				active = true;
			}
			else if (active && (!tranche1 || !tranche2 || !tranche3)) {
				digitalWrite(pinAutreLed, LOW); // On eteind l'autre LED
				active = false;
			}
		}

	} while (digitalRead(pinBouton)); // On continu tant qu'on ne relache pas le bouton
	delay(ANTI_REBONT); // Pour eviter des problèmes

	// On eteind les LEDs
	digitalWrite(pinLed, LOW);
	digitalWrite(pinAutreLed, LOW);

	return tempsCouleur; // On retourne le temps de l'equipe
}


//-----------------------------affichageBarre----------------------------------

void affichageBarre(unsigned short current) {
	// Fonction qui gère l'affichage de la barre de progression
	// On utilise le caractères speciaux defini dans character.h

	// La barre de progression à une longueur de 84 pixels, on utilise map
	// pour savoir où elle devrait être
	unsigned char progress;
	progress = map(current, 0, tempsCapture, 0, 84);
	progress = 84 - progress; // On l'inverse pour qu'il aille de 0 à 84

	// On calcule le nombre de bloc plein et le remplissage du bloc intermediaire
	unsigned char plein = progress / 6; // Il y'a 6 type de bloc possible
	unsigned char intermediaire = progress % 6;

	// On affiche tout
	lcd.setCursor(0,1);
	lcd.write(byte(0)); // On met le bloc de debut
	for (size_t i=0; i<plein; i++) lcd.write(byte(6)); // On met les blocs pleins
	lcd.write(byte(intermediaire+1)); // On met le bon bloc intermediaire
	for (size_t i=0; i<(13-plein); i++) lcd.write(byte(1)); // On complete avec les vides
	lcd.write(byte(7)); // On fini avec le bloc de fin
}


//--------------------------------finPartie------------------------------------

void finPartie(unsigned char pinBouton, unsigned char pinLed, unsigned char pinAutreLed) {
	//Affiche un message en fin de partie puis le gagnant avec le temps mis

	// On souhaite afficher plus tard le temps qu'à pris la partie, donc
	// on update tempsCourant maintenant
	tempsCourant = millis() - tempsDebut;


	// Affichage du texte
	lcd.clear();
	lcd.print("Drapeau capturee");
	lcd.setCursor(0, 1);
	lcd.print("  Equipe ");
	if (pinLed == RED_LED) lcd.print("Rouge");
	else if (pinLed == BLUE_LED) lcd.print("Bleue");

	// Animation son et LEDs

	tone(BUZZER, END_SOUND_FREQUENCY, 100);
	digitalWrite(pinAutreLed, HIGH);
	delay(55);
	digitalWrite(pinAutreLed, LOW);
	delay(55);
	digitalWrite(pinAutreLed, LOW);
	tone(BUZZER, END_SOUND_FREQUENCY, 200);
	digitalWrite(pinAutreLed, HIGH);
	delay(110);
	digitalWrite(pinAutreLed, LOW);
	delay(110);
	digitalWrite(pinAutreLed, HIGH);
	tone(BUZZER, END_SOUND_FREQUENCY, 200);
	digitalWrite(pinAutreLed, HIGH);
	delay(110);
	digitalWrite(pinAutreLed, LOW);
	delay(330);
	digitalWrite(pinAutreLed, LOW);
	tone(BUZZER, END_SOUND_FREQUENCY, 200);
	digitalWrite(pinAutreLed, HIGH);
	delay(110);
	digitalWrite(pinAutreLed, LOW);
	delay(110);
	digitalWrite(pinAutreLed, HIGH);
	tone(BUZZER, END_SOUND_FREQUENCY, 200);
	digitalWrite(pinAutreLed, HIGH);
	delay(110);
	digitalWrite(pinAutreLed, LOW);
	delay(330);
	digitalWrite(pinAutreLed, LOW);
	tone(BUZZER, END_SOUND_FREQUENCY, 200);
	digitalWrite(pinAutreLed, HIGH);
	delay(110);
	digitalWrite(pinAutreLed, LOW);
	delay(330);
	digitalWrite(pinAutreLed, HIGH);
	tone(BUZZER, END_SOUND_FREQUENCY, 200);
	digitalWrite(pinAutreLed, HIGH);
	delay(110);
	digitalWrite(pinAutreLed, LOW);
	delay(330);
	tone(BUZZER, END_SOUND_FREQUENCY, 2500);
	for(unsigned char i=255; i>0; i--) {
		analogWrite(pinAutreLed, i);
		delay(8);
	}
	digitalWrite(pinAutreLed, LOW);

	// On attend que l'equipe qui apppuyais sur son bouton le relache, puis on
	// attend un appui sur n'importe quel bouton
	waitButton(pinBouton);
	waitButtons();

	// Pour avoir le vrai temps

	// Affichage du texte
	lcd.clear();
	lcd.print("Gagnant : ");
	if (pinLed == RED_LED) lcd.print("Rouges");
	else if (pinLed == BLUE_LED) lcd.print(" Bleus");
	lcd.setCursor(0, 1);lcd.setCursor(0, 1);
	lcd.print("   h   min   s");
	updateTime();

	waitButtons();

	// A la fin, on redemarre l'arduino pour redemarer le jeu
	resetArduino();
}
