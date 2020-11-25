#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int suit; // 1 = ♠, 2 = ◇, 3 = ♡, 4 = ♣
	int number; // 1 = A, 11 = J, 12 = Q, 13 = K
} card_type;

typedef struct {
	card_type cards[52];
	int card_count;
} card_stack;


void shuffle_table(card_stack*);
void reset_cards(card_stack*);
card_type pick_card(card_stack*);
card_type pick_card_table(card_stack*);
char* card_to_string(card_type);
void print_card(card_type);
void new_round(card_stack*, card_stack*, card_stack*, int*);
void show_game_state(card_stack, card_stack);
int game_turn(card_stack*, int*, card_stack*, int*, card_stack*);
int get_card_sum(card_stack);


int main(void) {
	int player_money = 1000;
	card_stack table_cards;
	card_stack player_cards;
	card_stack dealer_cards;

	shuffle_table(&table_cards); //테이블에는 카드 1벌만 섞어놓음
	reset_cards(&player_cards);
	reset_cards(&dealer_cards);

	new_round(&table_cards, &player_cards, &dealer_cards, &player_money);

	return 0;
}

//테이블의 카드들을 생성
void shuffle_table(card_stack* table) {
	unsigned short seed = time(NULL);
	int added[52] = { 0, };

	for (int i = 0; i < 52;) {
		int card = rand() % 52;
		seed += rand();
		srand(seed); //무작위하게 생성된 시드값을 통해 랜덤 함수를 재설정
		if (added[card] == 0) {
			table->cards[i].suit = card / 13 + 1;
			table->cards[i].number = card % 13 + 1;
			added[card] = 1;
			i++;
		}
	}
	table->card_count = 52;

}

//카드 스택을 카드가 없는 상태로 초기화함
void reset_cards(card_stack* cards) {
	card_type blank = { 0, 0 };
	for (int i = 0; i < 52; i++) cards->cards[i] = blank;
	cards->card_count = 0;
}

//카드 스택에서 카드 1개를 뽑음
card_type pick_card(card_stack* stack) {
	if (stack->card_count <= 0) {
		printf("오류: 카드가 없습니다.\n"); //카드 스택에 카드가 없는 경우
		return;
	}

	card_type result = stack->cards[stack->card_count - 1];
	stack->card_count--;

	return result;
}

//카드 스택에서 카드 1개를 뽑음(테이블 전용)
card_type pick_card_table(card_stack* table) {
	if (table->card_count <= 0) {
		printf("테이블의 카드를 모두 소진했습니다. 새로운 카드 더미를 가져옵니다.\n"); //카드 스택에 카드가 없는 경우 새로 카드 1벌을 섞어 테이블에 놓음
		shuffle_table(table);
	}

	card_type result = table->cards[table->card_count - 1]; //컴파일시 경고(정상 작동?)
	table->card_count--;

	return result;
}

//카드 스택에 카드 1개를 넣음
void put_card(card_stack* deck, card_type card) {
	if (deck->card_count >= 52) {
		printf("오류: 카드를 넣을 공간이 없습니다.\n"); //카드 스택에 공간이 없는 경우
		return;
	}

	deck->cards[deck->card_count] = card;
	deck->card_count++;
}

//카드 데이터를 텍스트 형태로 반환
char* card_to_string(card_type card) {
	char resultsuit[5];
	char resultnum[3];

	//카드의 무늬를 문자열로 변환
	switch (card.suit) {
	case(1):
		strcpy(resultsuit, "♠");
		break;
	case(2):
		strcpy(resultsuit, "◇");
		break;
	case(3):
		strcpy(resultsuit, "♡");
		break;
	case(4):
		strcpy(resultsuit, "♣");
		break;
	}

	//카드의 숫자를 문자열로 변환
	if (card.number == 1) strcpy(resultnum, "A");
	else if (card.number >= 2 && card.number <= 10) _itoa(card.number, resultnum, 10);
	else if (card.number == 11) strcpy(resultnum, "J");
	else if (card.number == 12) strcpy(resultnum, "Q");
	else if (card.number == 13) strcpy(resultnum, "K");

	strcat(resultsuit, resultnum); //두 문자열을 합침

	return resultsuit;
}

//새로운 라운드를 플레이함.
void new_round(card_stack* table, card_stack* player, card_stack* dealer, int* money) {

	int bet = 0;
	int player_stay = 0;
	int dealer_stay = 0;

	reset_cards(player);
	reset_cards(dealer);

	while (1) {
		printf("베팅할 금액을 설정하세요. (현재 소지금: %d $): ", *money);
		scanf("%d", &bet);
		while (getchar() != '\n'); //정수 이외 입력 방지용(버퍼 비우기)
		if (bet <= 0) printf("0보다 큰 값을 설정해주세요.\n");
		else if (bet > * money) printf("소지금을 초과해서 베팅할 수 없습니다.\n");
		else break;
	}

	//플레이어와 딜러에게 카드를 2장씩 나누어줌
	put_card(player, pick_card_table(table));
	put_card(player, pick_card_table(table));
	put_card(dealer, pick_card_table(table));
	put_card(dealer, pick_card_table(table));

	int result = game_turn(player, &player_stay, dealer, &dealer_stay, table);
	//결괏값에 따른 분기
	switch (result) {
	case -3:
		printf("버스트! %d $만큼의 돈을 잃었습니다.\n\n", bet);
		*money -= bet;
		break;
	case -2:
		printf("딜러가 먼저 블랙잭을 달성했습니다. %d $만큼의 돈을 잃었습니다.\n\n", bet);
		*money -= bet;
		break;
	case -1:
		printf("딜러의 카드 합이 더 컸습니다. %d $만큼의 돈을 잃었습니다.\n\n", bet);
		*money -= bet;
		break;
	case 0:
		printf("양쪽의 카드 합이 같습니다. 베팅한 %d $는 다시 돌려받습니다.\n\n", bet);
		break;
	case 1:
		printf("플레이어의 카드 합이 더 컸습니다. %d $만큼의 돈을 땄습니다.\n\n", bet);
		*money += bet;
		break;
	case 2:
		printf("블랙잭! 베팅한 %d $의 2배인 %d $만큼의 돈을 땄습니다.\n\n", bet, bet * 2);
		*money += bet * 2;
		break;
	case 3:
		printf("딜러의 버스트! %d $만큼의 돈을 땄습니다.\n\n", bet);
		*money += bet;
		break;
	default:
		printf("오류: 정의되지 않은 값입니다.\n\n");
		break;
	}

	if (*money <= 0) {
		printf("돈이 모두 떨어졌습니다. 게임을 종료합니다.\n");
		exit(0);
	}
	else new_round(table, player, dealer, money); //재귀형으로 새로운 라운드 돌입
}

/*각각의 턴마다 플레이어와 딜러의 행동 설정
결괏값 1: 승리, 2: 블랙잭, 3: 딜러 버스트, -1: 패배, -2: 딜러 블랙잭, -3: 버스트, 0: 무승부*/
int game_turn(card_stack* player, int* player_stay, card_stack* dealer, int* dealer_stay, card_stack* table) {

	show_game_state(*player, *dealer);
	if (get_card_sum(*player) == 21) return 2; //빠른 블랙잭

	if (*player_stay == 0) {
		printf("h를 눌러 히트 하거나 s를 눌러 스탠드 하세요.\n");
		while (1) {
			char temp_hs = getchar();
			if (temp_hs == 'h') {
				printf("히트합니다.\n");
				while (getchar() != '\n'); //이후 문자는 버림
				put_card(player, pick_card_table(table));
				printf("새로 받은 카드: ");
				print_card(player->cards[player->card_count - 1]);
				printf("\n현재 카드의 합: %d\n\n", get_card_sum(*player));

				if (get_card_sum(*player) == 21) return 2; //블랙잭
				else if (get_card_sum(*player) > 21) return -3; //버스트
				break;
			}
			else if (temp_hs == 's') {
				printf("스탠드했습니다. \n\n");
				while (getchar() != '\n'); //이후 문자는 버림
				*player_stay = 1;

				if (*dealer_stay) {
					printf("플레이어의 카드 합: %d, 딜러의 카드 합: %d\n", get_card_sum(*player), get_card_sum(*dealer));
					if (get_card_sum(*player) > get_card_sum(*dealer)) return 1;
					else if (get_card_sum(*player) == get_card_sum(*dealer)) return 0;
					else return -1;
				}
				break;
			}
		}
	}
	if (*dealer_stay == 0) {
		if (get_card_sum(*dealer) == 21) return -2; //빠른 블랙잭

		if (get_card_sum(*dealer) <= 16) {
			//히트
			printf("딜러가 히트 했습니다.\n");
			put_card(dealer, pick_card_table(table));
			printf("새로 받은 카드: ");
			print_card(dealer->cards[dealer->card_count - 1]);
			printf("\n\n");

			if (get_card_sum(*dealer) == 21) return -2; //딜러 블랙잭
			else if (get_card_sum(*dealer) > 21) return 3; //딜러 버스트
		}
		else {
			//스탠드
			printf("딜러가 스탠드 했습니다.\n");
			*dealer_stay = 1;

			if (*player_stay) {
				printf("플레이어의 카드 합: %d, 딜러의 카드 합: %d\n", get_card_sum(*player), get_card_sum(*dealer));
				if (get_card_sum(*player) > get_card_sum(*dealer)) return 1;
				else if (get_card_sum(*player) == get_card_sum(*dealer)) return 0;
				else return -1;
			}
		}
	}

	int result = game_turn(player, player_stay, dealer, dealer_stay, table); //게임이 종료되지 않은 경우, 재귀적으로 다음 턴 시작
	return result;
}

//현재 플레이어와 딜러의 카드 상태를 보여줌
void show_game_state(card_stack player, card_stack dealer) {
	printf("플레이어의 카드: \n");
	for (int i = 0; i < player.card_count; i++) {
		print_card(player.cards[i]);
		printf(" ");
	}
	printf("\n합계 : %d\n", get_card_sum(player));

	printf("딜러의 카드: \n");
	printf("?? "); //딜러의 카드 1장은 보이지 않음
	for (int i = 1; i < dealer.card_count; i++) {
		print_card(dealer.cards[i]);
		printf(" ");
	}
	printf("\n\n");
}

//카드의 무늬와 숫자를 직접 출력함
void print_card(card_type card) {
	char res[5];
	strcpy(res, card_to_string(card));
	printf("%s", res);
}

//현재 카드 스택에 있는 카드 합을 블랙잭식으로 계산 후 반환함
int get_card_sum(card_stack deck) {
	int sum = 0, aces = 0;

	for (int i = 0; i < deck.card_count; i++) {
		if (deck.cards[i].number == 1) aces++; //에이스는 따로 계산
		else if (deck.cards[i].number > 10) sum += 10; //J, Q, K도 10으로 계산
		else sum += deck.cards[i].number;
	}

	for (; aces > 0; aces--) {
		if (aces == 1 && sum < 11) sum += 11; //마지막 에이스까지도 계산했음에도 합이 10 이하인 경우 해당 에이스는 11로 계산
		else sum += 1; //그 외의 경우 1로 계산
	}

	return sum;
}

// 카드 확인용, 카드 스택의 모든 카드를 출력함
void see_all_cards(card_stack cards) {
	for (int i = 0; i < 52; i++) {
		print_card(pick_card(&cards));
		printf(" ");
		if ((i + 1) % 5 == 0) printf("\n");
	}
}