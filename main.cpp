#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <math.h>

#include <string>
#include <vector>

extern "C" {
#include"./sdl/include/SDL.h"
#include"./sdl/include/SDL_main.h"
}
#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define GRUBOSC_RAMKI 20

//zrodla dodac
//jesli po lewej stronie sie odbije pilka - inny kat (bardziej w lewo)

struct Gra {
	int etap = 1;
	int punkty = 0;
	bool czy_przegrana = false;
};
Gra stan;
// narysowanie napisu txt na powierzchni screen, zaczynajπc od punktu (x, y)
// charset to bitmapa 128x128 zawierajπca znaki
//nie autorskie
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};

// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt úrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};


void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d≥ugoúci l w pionie (gdy dx = 0, dy = 1) 
// bπdü poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};


void rysuj_plansze(SDL_Surface *screen,int kolor_obrysu,int kolor_wypelnienia)
{
	
	//Lewa sciana
	DrawRectangle(screen, 0, 40, GRUBOSC_RAMKI, SCREEN_HEIGHT - 40, kolor_obrysu, kolor_wypelnienia);
	//Prawa sciana
	DrawRectangle(screen, SCREEN_WIDTH-20, 40, GRUBOSC_RAMKI, SCREEN_HEIGHT - 40, kolor_obrysu, kolor_wypelnienia);
	//GÛra
	DrawRectangle(screen, 0, 40, SCREEN_WIDTH, GRUBOSC_RAMKI, kolor_obrysu, kolor_wypelnienia);

}

class Paletka {
public:
	double x = SCREEN_WIDTH / 2;
	double y = SCREEN_HEIGHT - 20;
	double length = 150;
	double color;
	double speed = 200;

public:
	Paletka(int color) {
		this->color = color;
	}
	void rysuj(SDL_Surface* screen) {
		DrawRectangle(screen, this->x, this->y, this->length, 10, this->color, this->color);
	}
	void right(double delta) {
		/*Sprawdzenie czy nie jest na prawej scianie*/
		if (this->x + this->length + GRUBOSC_RAMKI < SCREEN_WIDTH)
			x += this->speed * delta;
		else {
			x = SCREEN_WIDTH - (this->length + GRUBOSC_RAMKI);
		}
	}

	void left(double delta) {
		/*Sprawdzenie czy nie wejdzie na lewa ramke*/
		if (this->x > GRUBOSC_RAMKI)
			x -= this->speed * delta;
		else {
			x = GRUBOSC_RAMKI;
		}
	}
};
class Pilka {
public:
	bool czy_w_ruchu = false;
	double x = 0;
	double y = SCREEN_HEIGHT - 30;
	double kat_x = 0.75;
	double kat_y = 0.25;
	double speed = 180;
private:
	SDL_Surface *ball;
public:
	Pilka() {
		this->ball = SDL_LoadBMP("./ball.bmp");
	}
	void rysuj(SDL_Surface* screen) {
		DrawSurface(screen, this->ball, this->x, this->y);
	}
	void move(double delta, Paletka paletka) {
		if (!czy_w_ruchu)
			return;
		x -= kat_x *  delta *speed;
		y -= kat_y * delta * speed;

		// Odbicie od gornej sciany 
		if (y < 65) 
		{
			kat_y *= -1;
			y = 65+0.1;
		}
		// Odbicie od prawej 
		if (this->x + GRUBOSC_RAMKI < SCREEN_WIDTH)
		{
			kat_x *= -1;
			//this->x = SCREEN_WIDTH - GRUBOSC_RAMKI;
		}
		//odbicie lewa 
		if (this->x > GRUBOSC_RAMKI)
		{
			kat_x *= -1;
		}

		// Sprawdzenie czy pilka znajduje sie na wysokosci paletki 
		if (this->y > paletka.y-3 && this->y<paletka.y + 3) {//y pilki musi byc ponad(czyli mniejsze) dolna granica paletki
			
			if (this->x > paletka.x && this->x < paletka.x + paletka.length) {
				odbicie_poziomo();
			}
		}
	}

	void odbicie_poziomo() {
		kat_y *= -1;
	}
	void odbicie_pion() {
		kat_x *= -1;
	}
	
};

class Klocek {
	public:
	double x;
	double y;
	double length_X = 40;
	double length_Y = 20;
	int color;
	int color_rama;
public:
	Klocek(int x, int y, int color, int color_rama) {
		this->x = x+15;
		this->y = y-20;
		this->color = color;
		this->color_rama = color_rama;
	}
	void rysuj(SDL_Surface* screen) {
		DrawRectangle(screen, this->x, this->y, this->length_X, this->length_Y, this->color, this->color_rama);
	}
};
class KonternerKlockow {
public:
	Klocek* klocki[100];
	KonternerKlockow() {
		for (int i = 0; i < 100; i++) {
			klocki[i] = NULL;
		}
	}
	void add(Klocek *klocek) {
		int i = 0;
		while (klocki[i] != NULL) { 
			i++;
		}
		if (i < 100) {
			klocki[i] = klocek;
		}
	}
	void rysuj(SDL_Surface* screen) {
		for (int i = 0; i < 100; i++) {
			if (klocki[i] != NULL) {
				klocki[i]->rysuj(screen);
			}
		}
	}
	int getcount() { //ile jest klockow w grze
		int count = 0;
		for (int i = 0; i < 100; i++)
			if (klocki[i] != NULL)
				count++;
		return count;
	}

	void sprawdzKolizje(Pilka &pilka, Gra gra) {
		for (int i = 0; i < 100; i++) {
			if (klocki[i] != NULL) {
				/*Uderzenie w dolna sciane klocka*/
				if (int(pilka.y - 3) == klocki[i]->y + klocki[i]->length_Y) {//wysookosc
					if (int(pilka.x) >= klocki[i]->x && int(pilka.x) <= klocki[i]->x + klocki[i]->length_X) { 
						delete klocki[i];
						klocki[i] = NULL;
						pilka.odbicie_poziomo();
						stan.punkty += 10;
						break;
					}
				}

				/*Uderzenie w gorna sciane klocka*/
				if (int(pilka.y + 3) == klocki[i]->y) {
					if (int(pilka.x) >= klocki[i]->x && int(pilka.x) <= klocki[i]->x + klocki[i]->length_X) {
						delete klocki[i];
						klocki[i] = NULL;
						pilka.odbicie_poziomo();
						stan.punkty += 10;
						break;
					}
				}

				/*Uderzenie w lawa sciane klocka*/
				if (int(pilka.x + 3) == klocki[i]->x) {
					if (int(pilka.y) >= klocki[i]->y && int(pilka.y) <= klocki[i]->y + klocki[i]->length_Y) {
						delete klocki[i];
						klocki[i] = NULL;
						pilka.odbicie_pion();
						stan.punkty += 10;

						break;
					}
				}
				/*Uderzenie w prawa sciane klocka*/
				if (int(pilka.x - 3) == klocki[i]->x+ klocki[i]->length_X) {
					if (int(pilka.y) >= klocki[i]->y && int(pilka.y) <= klocki[i]->y + klocki[i]->length_Y) {
						delete klocki[i];
						klocki[i] = NULL;
						pilka.odbicie_pion();
						stan.punkty += 10;

						break;
					}
				}
			}
		}
	}
};
// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	srand(time(NULL));
	
	double t1, t2;
	int quit, rc;
	double delta, worldTime, fps;
	SDL_Event event;
	SDL_Surface *screen, *charset;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	/*Sprawdza czy sdl sie inicjalizuje*/
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "s14222");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);

	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int dirty_dulled_red = SDL_MapRGB(screen->format, 0x99, 0x42, 0x4f);
	int bright_pink = SDL_MapRGB(screen->format, 0xff, 0xc7, 0xc7);
	int yellow = SDL_MapRGB(screen->format, 0xfe, 0xa1, 0x03);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int burgundy = SDL_MapRGB(screen->format, 0x4d, 0x02, 0x2a);
	int color_klocek_typ_1 = SDL_MapRGB(screen->format, 0x33, 0x33, 0xCC);
	int color_klocek_typ_1_rama = SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xCC);
	Paletka paletka(dirty_dulled_red);
	Pilka pilka;
	KonternerKlockow klocki;

	SDL_ShowCursor(SDL_DISABLE);

	charset = SDL_LoadBMP("./cs8x8.bmp");

	if(charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(charset, true, 0x000000);
	char text[128];
	
	t1 = SDL_GetTicks(); //number of milliseconds since the SDL library initialization
	fps = 0;
	quit = 0;
	worldTime = 0;
	int eventx = -1;
	
	for(int j=0;j<8;j++)
	for (int i = 0; i < 11; i++) {
		klocki.add(new Klocek(80 + i*40, 200+j*20, czarny, bright_pink));
	}
	
	while (!quit) {
		t2 = SDL_GetTicks();

		delta = (t2 - t1) * 0.001;
		t1 = t2;
		worldTime += delta;

		double fpsHelper = 1.0;
		fpsHelper /= delta;
		fps = fpsHelper;

		if (klocki.getcount() == 0) {
			for (int j = 0; j<8; j++)
				for (int i = 0; i < 11; i++) {
					if(std::rand() %2 ==0) 
						klocki.add(new Klocek(80 + i * 40, 200 + j * 20, czarny, bright_pink));
				}
		}

		if (worldTime > 60 && stan.etap == 1)
		{
			pilka.speed = 300;
			stan.etap = 2;
			paletka.speed += 50;
		}
		if (worldTime > 120 && stan.etap == 2)
		{
			pilka.speed = 350;
			stan.etap = 3;
			paletka.speed += 50;
			paletka.length -= 50;
		}
		// Czy pilka znajduje sie pod paletka
		if (pilka.y > 475) {
			stan.czy_przegrana = true;
		}
			
		SDL_FillRect(screen, NULL, czarny);
		if (stan.czy_przegrana) {
			DrawRectangle(screen, 0, 4, SCREEN_WIDTH, 36, czerwony, burgundy);
			sprintf(text, "Przegrales");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
		}
		else
		{
			rysuj_plansze(screen, burgundy, burgundy);

			paletka.rysuj(screen);
			//srodek paletki
			if (!pilka.czy_w_ruchu) {
				pilka.x = paletka.x + paletka.length / 2;
			}

			pilka.rysuj(screen);
			pilka.move(delta, paletka);

			klocki.rysuj(screen);
			klocki.sprawdzKolizje(pilka, stan);

			DrawRectangle(screen, 0, 4, SCREEN_WIDTH, 36, czerwony, burgundy);
			sprintf(text, "Etap %d Punkty: %d Czas trwania = %.1lf s  %.0lf klatek / s", stan.etap, stan.punkty, worldTime, fps);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
			sprintf(text, "Esc - wyjscie, spacja - start, r - restart");
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);


			/*Paletka nie wchodzi na ramki i przesuniecie*/
			if (eventx == SDLK_RIGHT) {
				paletka.right(delta);
			}
			else if (eventx == SDLK_LEFT) {
				paletka.left(delta);
			}
		}
		/*wyswietlenie zaktualizowanego screenu*/
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL);
		SDL_RenderPresent(renderer);
	
		while(SDL_PollEvent(&event)) {
			switch(event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						quit = 1;
					} 
					else if (event.key.keysym.sym == SDLK_r) {
						pilka = Pilka();
						klocki = KonternerKlockow();
						for (int j = 0; j<8; j++)
							for (int i = 0; i < 11; i++) {
								klocki.add(new Klocek(80 + i * 40, 200 + j * 20, czarny, bright_pink));
							}
						
						stan.czy_przegrana = false;
						worldTime = 0;
						stan.etap = 1;
						stan.punkty = 0;
					}
					else if (event.key.keysym.sym == SDLK_SPACE) {
						pilka.czy_w_ruchu = true;
					}
					else {
						eventx = event.key.keysym.sym; //paletka
					}
					break;
				case SDL_KEYUP:
					eventx = -1;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_PRESSED:
					break;
				};
			};
		};

	// zwolnienie powierzchni
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
