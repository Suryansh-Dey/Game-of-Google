#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//**** ADJUSTABLE CONSTANT PARAMETERS ****
constexpr int FPS = 40;
//**** GAME CONSTANTS ****
constexpr int FRAME_GAP = 1000 / FPS;
std::unordered_map<std::string, SDL_Texture *> images;
int SCREEN_WIDTH = 0, SCREEN_HEIGHT = 0;
SDL_Window *window;

void FPS_manager(const int FRAME_GAP)
{
	static int last_time = 0;
	int delay = SDL_GetTicks() - last_time;
	if (delay < FRAME_GAP)
	{
		SDL_Delay(FRAME_GAP - delay);
	}
	last_time = SDL_GetTicks();
}
SDL_Renderer *createWindow(int &SCREEN_WIDTH, int &SCREEN_HEIGHT, SDL_Window *&window)
{
	SDL_Window *temp_window = SDL_CreateWindow("SCANNING DISPLAY DIMENTION", 0, 0, 0, 0, SDL_WINDOW_SHOWN);
	SDL_GetWindowSize(temp_window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
	SDL_DestroyWindow(temp_window);
	window = SDL_CreateWindow("GAME OF GOOGLE", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	SDL_Renderer *s = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_RenderClear(s);
	return s;
}
inline SDL_Rect createRect(int x, int y, int w, int h)
{
	SDL_Rect rect{x - w / 2, y - h / 2, w, h};
	return rect;
}
inline bool isColliding(SDL_Rect rect, int x, int y)
{
	return abs(x - rect.w / 2 - rect.x) < rect.w / 2 and abs(y - rect.h / 2 - rect.y) < rect.h / 2;
}
class showNum
{
	static constexpr int MAX_DIGIT_COUNT = 20;
	SDL_Texture *digits_img;
	SDL_Point img_dimention;
	SDL_Rect digit_rect;
	SDL_Rect digit_srcRect;
	int x, y, limit;

  public:
	void set_rect(int x, int y, int w, int h)
	{
		this->x = x;
		this->y = y;
		digit_rect = SDL_Rect{x, y, w, h};
	}
	showNum() {}
	showNum(SDL_Texture *arg_digits_img, int limit) : limit(limit)
	{
		digits_img = arg_digits_img;
		SDL_QueryTexture(arg_digits_img, NULL, NULL, &img_dimention.x, &img_dimention.y);
		img_dimention.x /= 10;
	}
	void blit(SDL_Renderer *s, long long num)
	{
		if (!num)
		{
			digit_srcRect =
				SDL_Rect{0, 0, img_dimention.x, img_dimention.y};
			digit_rect.x = x - digit_rect.w / 2;
			SDL_RenderCopy(s, digits_img, &digit_srcRect, &digit_rect);
			return;
		}
		//converting int to array of int with each digit as it's element
		char digits[MAX_DIGIT_COUNT];
		int i = 1;
		for (; num; i++)
		{
			long long num2 = num;
			num /= 10;
			digits[i] = num2 - num * 10;
		}
		digits[0] = i; //storing array length in 0th index

		if (digits[0] > limit)
			digit_rect.x = x - digit_rect.w * limit / 2;
		else
			digit_rect.x = x - digit_rect.w * (digits[0] - 1) / 2;
		for (int i = digits[0] - 1; i > 0; i--)
		{
			digit_srcRect = SDL_Rect{img_dimention.x * digits[i], 0, img_dimention.x, img_dimention.y};
			digit_rect.x += (-digit_rect.x + x - digit_rect.w * (limit) / 2) * not(((digits[0] - 1 - i) % limit) or i == digits[0] - 1);
			digit_rect.y = y + digit_rect.h * ((digits[0] - 1 - i) / limit) * 1.1;
			SDL_RenderCopy(s, digits_img, &digit_srcRect, &digit_rect);
			digit_rect.x += digit_rect.w;
		}
	}
};
class Card
{
	friend std::vector<Card> restart(SDL_Renderer *);

  public:
	static constexpr float RETARDATION = 1, CARD_WIDTH_RATIO = 0.5, CARD_HEIGHT_RATIO = 0.5, CARD_X_RATIO = 0.5, CARD_Y_RATIO = 0.5, GAP_RATIO = 0.25, MAX_ENLARGEMENT_RATIO = 0.5;
	static constexpr int FLIPPING_TIME = 20, LINE_DIGITS_COUNT = 8;

  private:
	int SCREEN_WIDTH, SCREEN_HEIGHT;
	bool flipped = 0;
	int x, y, cardNumber, flippingRemaining = FLIPPING_TIME;
	long long numberValue;
	static int velocity;
	SDL_Texture *unflippedCardImage, *flippedCardImage;
	std::vector<SDL_Texture *> flippedFrames, unflippedFrames;
	showNum number;
	static int cardCount;
	static long long highestNumber;

  public:
	Card() {}
	Card(int SCREEN_WIDTH, int SCREEN_HEIGHT, std::unordered_map<std::string, SDL_Texture *> &images) : SCREEN_WIDTH(SCREEN_WIDTH), SCREEN_HEIGHT(SCREEN_HEIGHT), number(images["digits"], LINE_DIGITS_COUNT - 1)
	{
		this->x = SCREEN_WIDTH * Card::cardCount * (CARD_X_RATIO + GAP_RATIO) + SCREEN_WIDTH * CARD_WIDTH_RATIO / 2;
		this->y = SCREEN_HEIGHT * CARD_Y_RATIO;
		this->unflippedFrames = std::vector<SDL_Texture *>{images["flippingCard_1"], images["flippingCard_2"], images["flippingCard_3"]};
		this->flippedFrames = std::vector<SDL_Texture *>{images["flippingCard_4"], images["flippingCard_5"], images["flippingCard_6"]};
		this->unflippedCardImage = images["unflippedCard"];
		this->flippedCardImage = images["flippedCard"];
		this->numberValue = ((long long)(rand()) * rand()) / pow((long long)(10), (long long)(rand() % 18));
		if (numberValue > highestNumber)
			highestNumber = numberValue;
		cardNumber = Card::cardCount;
		Card::cardCount++;
	}
	inline int get_x();
	inline static int get_velocity();
	inline bool isUnflipped();
	inline bool flip();
	void update();
	static void updateVelocity();
	inline static void slide(int velocity);
	void render(SDL_Renderer *s);
};
int Card::cardCount = 0;
long long Card::highestNumber = 0;
int Card::velocity = 0;
inline int Card::get_x()
{
	return this->x;
}
inline int Card::get_velocity()
{
	return Card::velocity;
}
inline bool Card::isUnflipped()
{
	return not this->flipped;
}
inline bool Card::flip()
{
	this->flipped = 1;
	return numberValue == Card::highestNumber;
}
void Card::update()
{
	this->x += Card::velocity;
}
void Card::updateVelocity()
{
	if (abs(velocity) >= RETARDATION)
		Card::velocity -= (abs(Card::velocity) / Card::velocity) * RETARDATION;
	else
		Card::velocity = 0;
}
inline void Card::slide(int velocity)
{
	Card::velocity = velocity;
}
void Card::render(SDL_Renderer *s)
{
	this->flippingRemaining -= (this->flipped and flippingRemaining > 0);
	if (this->x < -CARD_WIDTH_RATIO * SCREEN_WIDTH or this->x > SCREEN_WIDTH + CARD_WIDTH_RATIO * SCREEN_WIDTH)
		return;
	number.set_rect(x, y + CARD_HEIGHT_RATIO * SCREEN_HEIGHT / 2, CARD_WIDTH_RATIO * SCREEN_WIDTH / 5, CARD_WIDTH_RATIO * SCREEN_WIDTH / 5);
	number.blit(s, cardNumber + 1);
	if (this->flippingRemaining == FLIPPING_TIME)
	{
		SDL_Rect tempRect = createRect(x, y, CARD_WIDTH_RATIO * SCREEN_WIDTH, CARD_HEIGHT_RATIO * SCREEN_HEIGHT);
		SDL_RenderCopy(s, this->unflippedCardImage, NULL, &tempRect);
	}
	else if (this->flippingRemaining == 0)
	{
		SDL_Rect tempRect = createRect(x, y, CARD_WIDTH_RATIO * SCREEN_WIDTH, CARD_HEIGHT_RATIO * SCREEN_HEIGHT);
		SDL_RenderCopy(s, this->flippedCardImage, NULL, &tempRect);
		number.set_rect(x, y - tempRect.h / 3, tempRect.w / LINE_DIGITS_COUNT, tempRect.w / LINE_DIGITS_COUNT);
		number.blit(s, numberValue);
	}
	else if (this->flippingRemaining > FLIPPING_TIME / 2)
	{
		float enlargement = 1 + MAX_ENLARGEMENT_RATIO * float(FLIPPING_TIME - this->flippingRemaining) / (FLIPPING_TIME / 2);
		SDL_Rect tempRect = createRect(x, y, enlargement * SCREEN_WIDTH * CARD_WIDTH_RATIO * (this->flippingRemaining - FLIPPING_TIME / 2) / (FLIPPING_TIME / 2), enlargement * CARD_HEIGHT_RATIO * SCREEN_HEIGHT);
		SDL_RenderCopy(s, unflippedFrames[unflippedFrames.size() * (FLIPPING_TIME - this->flippingRemaining) / (0.5 + FLIPPING_TIME / 2)], NULL, &tempRect);
	}
	else
	{
		float enlargement = 1 + MAX_ENLARGEMENT_RATIO * this->flippingRemaining / (FLIPPING_TIME / 2);
		SDL_Rect tempRect = createRect(x, y, enlargement * SCREEN_WIDTH * CARD_WIDTH_RATIO * ((FLIPPING_TIME / 2) - this->flippingRemaining) / (FLIPPING_TIME / 2), enlargement * CARD_HEIGHT_RATIO * SCREEN_HEIGHT);
		SDL_RenderCopy(s, flippedFrames[flippedFrames.size() * ((FLIPPING_TIME / 2) - this->flippingRemaining) / (0.5 + FLIPPING_TIME / 2)], NULL, &tempRect);
		number.set_rect(x, y - tempRect.h / 3, tempRect.w / LINE_DIGITS_COUNT, CARD_WIDTH_RATIO * SCREEN_WIDTH / LINE_DIGITS_COUNT);
		number.blit(s, numberValue);
	}
}
int retryAngle;
bool win, found, over, correctPassed, pressed = true;
int8_t type = 0;
std::vector<Card> restart(SDL_Renderer *s)
{
	static constexpr int K = 100, PRESSING_THRESHOLD = 150;
	static float totalCards = 10;
	Card::cardCount = 0;
	Card::highestNumber = 0;
	Card::velocity = 0;
	long long startTime = 0;
	SDL_Rect increaseRect = createRect(SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.5, SCREEN_HEIGHT * 0.25, SCREEN_HEIGHT * 0.25);
	SDL_Rect decreaseRect = createRect(SCREEN_WIDTH * 0.1, SCREEN_HEIGHT * 0.75, SCREEN_HEIGHT * 0.25, SCREEN_HEIGHT * 0.25);
	SDL_Rect startRect = createRect(SCREEN_WIDTH * 0.9, SCREEN_HEIGHT * 0.5, SCREEN_HEIGHT * 0.25, SCREEN_HEIGHT * 0.25);
	SDL_Rect cardsRect = createRect(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, SCREEN_WIDTH * 0.7, SCREEN_HEIGHT * 0.5);
	showNum number(images["digits"], 4);
	number.set_rect(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5 - SCREEN_HEIGHT * 0.25 / 2, SCREEN_HEIGHT * 0.25, SCREEN_HEIGHT * 0.25);
	while (1)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_FINGERDOWN:
				if (isColliding(increaseRect, event.tfinger.x * SCREEN_WIDTH, event.tfinger.y * SCREEN_HEIGHT))
				{
					startTime = SDL_GetTicks();
					type = 1;
					pressed = false;
				}
				else if (isColliding(decreaseRect, event.tfinger.x * SCREEN_WIDTH, event.tfinger.y * SCREEN_HEIGHT))
				{
					startTime = SDL_GetTicks();
					type = -1;
					pressed = false;
				}
				else if (isColliding(startRect, event.tfinger.x * SCREEN_WIDTH, event.tfinger.y * SCREEN_HEIGHT))
					goto start;
				break;
			case SDL_FINGERUP:
				if (isColliding(increaseRect, event.tfinger.x * SCREEN_WIDTH, event.tfinger.y * SCREEN_HEIGHT) or isColliding(decreaseRect, event.tfinger.x * SCREEN_WIDTH, event.tfinger.y * SCREEN_HEIGHT))
					startTime = 0;
				break;
			case SDL_QUIT:
				SDL_Quit();
			}
		}
		if (startTime)
		{
			if (SDL_GetTicks() - startTime > PRESSING_THRESHOLD)
				totalCards += type * float(sqrt(SDL_GetTicks() - startTime - PRESSING_THRESHOLD)) / K;
			else if (not pressed)
			{
				totalCards += type;
				pressed = true;
			}
			totalCards += (-totalCards + 1) * (totalCards < 1);
		}
		SDL_RenderCopy(s, images["cards"], NULL, &cardsRect);
		SDL_RenderCopy(s, images["increase"], NULL, &increaseRect);
		SDL_RenderCopy(s, images["decrease"], NULL, &decreaseRect);
		SDL_RenderCopy(s, images["start"], NULL, &startRect);
		number.blit(s, totalCards);
		SDL_RenderPresent(s);
		SDL_RenderClear(s);
		FPS_manager(FRAME_GAP);
	}
start:
	std::vector<Card> cards(totalCards);
	for (Card &card : cards)
		card = Card(SCREEN_WIDTH, SCREEN_HEIGHT, images);
	retryAngle = 0, win = 0, found = 0, over = 0, correctPassed = 0;
	return cards;
}
int main()
{
	SDL_Renderer *s = createWindow(SCREEN_WIDTH, SCREEN_HEIGHT, window);
	while (SCREEN_WIDTH < SCREEN_HEIGHT)
	{
		SDL_DestroyRenderer(s);
		SDL_DestroyWindow(window);
		s = createWindow(SCREEN_WIDTH, SCREEN_HEIGHT, window);
		SDL_RenderCopy(s, IMG_LoadTexture(s, "resources/landscapeMode.jpeg"), NULL, NULL);
		SDL_RenderPresent(s);
	}
	//****LOADING RESOURCES****
	images["unflippedCard"] = IMG_LoadTexture(s, "resources/unflippedCard.png");
	images["flippedCard"] = IMG_LoadTexture(s, "resources/flippedCard.png");
	images["flippingCard_1"] = IMG_LoadTexture(s, "resources/flippingCard_1.png");
	images["flippingCard_2"] = IMG_LoadTexture(s, "resources/flippingCard_2.png");
	images["flippingCard_3"] = IMG_LoadTexture(s, "resources/flippingCard_3.png");
	images["flippingCard_4"] = IMG_LoadTexture(s, "resources/flippingCard_4.png");
	images["flippingCard_5"] = IMG_LoadTexture(s, "resources/flippingCard_5.png");
	images["flippingCard_6"] = IMG_LoadTexture(s, "resources/flippingCard_6.png");
	images["digits"] = IMG_LoadTexture(s, "resources/digits.png");
	images["retry"] = IMG_LoadTexture(s, "resources/retry.png");
	images["I_QUIT"] = IMG_LoadTexture(s, "resources/I_QUIT.png");
	SDL_SetTextureAlphaMod(images["I_QUIT"], 155);
	images["win"] = IMG_LoadTexture(s, "resources/win.png");
	images["lose"] = IMG_LoadTexture(s, "resources/lose.png");
	images["increase"] = IMG_LoadTexture(s, "resources/increase.png");
	images["decrease"] = IMG_LoadTexture(s, "resources/decrease.png");
	images["start"] = IMG_LoadTexture(s, "resources/start.png");
	images["arrow"] = IMG_LoadTexture(s, "resources/arrow.png");
	images["cards"] = IMG_LoadTexture(s, "resources/cards.png");

	SDL_Rect retryRect = SDL_Rect{0, 10, SCREEN_HEIGHT / 10, SCREEN_HEIGHT / 10};
	SDL_Rect I_QUITRect = retryRect;
	I_QUITRect.w *= 2;
	I_QUITRect.x = SCREEN_WIDTH - I_QUITRect.w * 1.1;
	SDL_Rect arrowRect = SDL_Rect{0, int(SCREEN_HEIGHT * 0.75), int(SCREEN_HEIGHT * 0.25), int(SCREEN_HEIGHT * 0.25 / 2)};
	SDL_Rect winRect = createRect(SCREEN_WIDTH / 2, SCREEN_HEIGHT * 0.25, SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5);
restarting:
	std::vector<Card> cards = restart(s);
	float dx = 0;
	while (1)
	{
		float touch_x = 0, touch_y;
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_FINGERDOWN:
				touch_x = event.tfinger.x * SCREEN_WIDTH;
				touch_y = event.tfinger.y * SCREEN_HEIGHT;
				if (isColliding(retryRect, touch_x, touch_y))
					goto restarting;
				else if (isColliding(I_QUITRect, touch_x, touch_y))
					SDL_SetTextureAlphaMod(images["I_QUIT"], 255);
				break;
			case SDL_FINGERUP:
				SDL_SetTextureAlphaMod(images["I_QUIT"], 155);
				if (isColliding(I_QUITRect, event.tfinger.x * SCREEN_WIDTH, event.tfinger.y * SCREEN_HEIGHT) and not over)
				{
					over = true;
					win = found;
				}
				dx = 0;
				break;
			case SDL_FINGERMOTION:
				if (abs(event.tfinger.dx) > abs(dx) or (event.tfinger.dx * SCREEN_WIDTH < 1) or event.tfinger.dx * Card::get_velocity() < 0)
					dx = event.tfinger.dx;
				break;
			case SDL_QUIT:
				SDL_Quit();
			default:
				dx = 0;
			}
		}
		if (cards[0].get_x() > SCREEN_WIDTH)
			Card::slide(SCREEN_WIDTH - cards[0].get_x());
		else if (cards[cards.size() - 1].get_x() < 0)
			Card::slide(-cards[cards.size() - 1].get_x());
		else if (dx)
			Card::slide(dx * SCREEN_WIDTH);
		for (Card &card : cards)
		{
			if (touch_x and abs(card.get_x() - touch_x) < Card::CARD_WIDTH_RATIO * SCREEN_WIDTH / 2 and abs(Card::CARD_Y_RATIO * SCREEN_HEIGHT - touch_y) < Card::CARD_HEIGHT_RATIO * SCREEN_HEIGHT / 2)
			{
				if (correctPassed and card.isUnflipped())
					over = 1;
				else
					found |= card.flip();
			}
			card.update();
			card.render(s);
		}
		Card::updateVelocity();
		correctPassed = found;
		if (over)
		{
			for (Card &card : cards)
			{
				if (card.flip())
				{
					if (card.get_x() < 0)
					{
						arrowRect.x = 0;
						if (touch_x and isColliding(arrowRect, touch_x, touch_y))
							Card::slide(sqrt(-2 * (card.get_x() - SCREEN_WIDTH / 2) * Card::RETARDATION));
						SDL_RenderCopyEx(s, images["arrow"], NULL, &arrowRect, 180, NULL, SDL_FLIP_NONE);
					}
					else if (card.get_x() > SCREEN_WIDTH)
					{
						arrowRect.x = SCREEN_WIDTH - arrowRect.w;
						if (touch_x and isColliding(arrowRect, touch_x, touch_y))
							Card::slide(-sqrt(2 * (card.get_x() - SCREEN_WIDTH / 2) * Card::RETARDATION));
						SDL_RenderCopy(s, images["arrow"], NULL, &arrowRect);
					}
					else
					{
						arrowRect.x = card.get_x() - arrowRect.w / 2;
						SDL_RenderCopyEx(s, images["arrow"], NULL, &arrowRect, -90, NULL, SDL_FLIP_NONE);
					}
				}
			}
			retryAngle -= 1;
			if (win)
				SDL_RenderCopy(s, images["win"], NULL, &winRect);
			else
				SDL_RenderCopy(s, images["lose"], NULL, &winRect);
			SDL_RenderCopyEx(s, images["retry"], NULL, &retryRect, retryAngle, NULL, SDL_FLIP_NONE);
		}
		else
			SDL_RenderCopy(s, images["retry"], NULL, &retryRect);
		SDL_RenderCopy(s, images["I_QUIT"], NULL, &I_QUITRect);
		SDL_RenderPresent(s);
		SDL_RenderClear(s);
		FPS_manager(FRAME_GAP);
	}
	return 0;
}