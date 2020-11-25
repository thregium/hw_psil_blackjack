#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int suit; // 1 = ��, 2 = ��, 3 = ��, 4 = ��
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

	shuffle_table(&table_cards); //���̺��� ī�� 1���� �������
	reset_cards(&player_cards);
	reset_cards(&dealer_cards);

	new_round(&table_cards, &player_cards, &dealer_cards, &player_money);

	return 0;
}

//���̺��� ī����� ����
void shuffle_table(card_stack* table) {
	unsigned short seed = time(NULL);
	int added[52] = { 0, };

	for (int i = 0; i < 52;) {
		int card = rand() % 52;
		seed += rand();
		srand(seed); //�������ϰ� ������ �õ尪�� ���� ���� �Լ��� �缳��
		if (added[card] == 0) {
			table->cards[i].suit = card / 13 + 1;
			table->cards[i].number = card % 13 + 1;
			added[card] = 1;
			i++;
		}
	}
	table->card_count = 52;

}

//ī�� ������ ī�尡 ���� ���·� �ʱ�ȭ��
void reset_cards(card_stack* cards) {
	card_type blank = { 0, 0 };
	for (int i = 0; i < 52; i++) cards->cards[i] = blank;
	cards->card_count = 0;
}

//ī�� ���ÿ��� ī�� 1���� ����
card_type pick_card(card_stack* stack) {
	if (stack->card_count <= 0) {
		printf("����: ī�尡 �����ϴ�.\n"); //ī�� ���ÿ� ī�尡 ���� ���
		return;
	}

	card_type result = stack->cards[stack->card_count - 1];
	stack->card_count--;

	return result;
}

//ī�� ���ÿ��� ī�� 1���� ����(���̺� ����)
card_type pick_card_table(card_stack* table) {
	if (table->card_count <= 0) {
		printf("���̺��� ī�带 ��� �����߽��ϴ�. ���ο� ī�� ���̸� �����ɴϴ�.\n"); //ī�� ���ÿ� ī�尡 ���� ��� ���� ī�� 1���� ���� ���̺� ����
		shuffle_table(table);
	}

	card_type result = table->cards[table->card_count - 1]; //�����Ͻ� ���(���� �۵�?)
	table->card_count--;

	return result;
}

//ī�� ���ÿ� ī�� 1���� ����
void put_card(card_stack* deck, card_type card) {
	if (deck->card_count >= 52) {
		printf("����: ī�带 ���� ������ �����ϴ�.\n"); //ī�� ���ÿ� ������ ���� ���
		return;
	}

	deck->cards[deck->card_count] = card;
	deck->card_count++;
}

//ī�� �����͸� �ؽ�Ʈ ���·� ��ȯ
char* card_to_string(card_type card) {
	char resultsuit[5];
	char resultnum[3];

	//ī���� ���̸� ���ڿ��� ��ȯ
	switch (card.suit) {
	case(1):
		strcpy(resultsuit, "��");
		break;
	case(2):
		strcpy(resultsuit, "��");
		break;
	case(3):
		strcpy(resultsuit, "��");
		break;
	case(4):
		strcpy(resultsuit, "��");
		break;
	}

	//ī���� ���ڸ� ���ڿ��� ��ȯ
	if (card.number == 1) strcpy(resultnum, "A");
	else if (card.number >= 2 && card.number <= 10) _itoa(card.number, resultnum, 10);
	else if (card.number == 11) strcpy(resultnum, "J");
	else if (card.number == 12) strcpy(resultnum, "Q");
	else if (card.number == 13) strcpy(resultnum, "K");

	strcat(resultsuit, resultnum); //�� ���ڿ��� ��ħ

	return resultsuit;
}

//���ο� ���带 �÷�����.
void new_round(card_stack* table, card_stack* player, card_stack* dealer, int* money) {

	int bet = 0;
	int player_stay = 0;
	int dealer_stay = 0;

	reset_cards(player);
	reset_cards(dealer);

	while (1) {
		printf("������ �ݾ��� �����ϼ���. (���� ������: %d $): ", *money);
		scanf("%d", &bet);
		while (getchar() != '\n'); //���� �̿� �Է� ������(���� ����)
		if (bet <= 0) printf("0���� ū ���� �������ּ���.\n");
		else if (bet > * money) printf("�������� �ʰ��ؼ� ������ �� �����ϴ�.\n");
		else break;
	}

	//�÷��̾�� �������� ī�带 2�徿 ��������
	put_card(player, pick_card_table(table));
	put_card(player, pick_card_table(table));
	put_card(dealer, pick_card_table(table));
	put_card(dealer, pick_card_table(table));

	int result = game_turn(player, &player_stay, dealer, &dealer_stay, table);
	//�ᱣ���� ���� �б�
	switch (result) {
	case -3:
		printf("����Ʈ! %d $��ŭ�� ���� �Ҿ����ϴ�.\n\n", bet);
		*money -= bet;
		break;
	case -2:
		printf("������ ���� ������ �޼��߽��ϴ�. %d $��ŭ�� ���� �Ҿ����ϴ�.\n\n", bet);
		*money -= bet;
		break;
	case -1:
		printf("������ ī�� ���� �� �ǽ��ϴ�. %d $��ŭ�� ���� �Ҿ����ϴ�.\n\n", bet);
		*money -= bet;
		break;
	case 0:
		printf("������ ī�� ���� �����ϴ�. ������ %d $�� �ٽ� �����޽��ϴ�.\n\n", bet);
		break;
	case 1:
		printf("�÷��̾��� ī�� ���� �� �ǽ��ϴ�. %d $��ŭ�� ���� �����ϴ�.\n\n", bet);
		*money += bet;
		break;
	case 2:
		printf("����! ������ %d $�� 2���� %d $��ŭ�� ���� �����ϴ�.\n\n", bet, bet * 2);
		*money += bet * 2;
		break;
	case 3:
		printf("������ ����Ʈ! %d $��ŭ�� ���� �����ϴ�.\n\n", bet);
		*money += bet;
		break;
	default:
		printf("����: ���ǵ��� ���� ���Դϴ�.\n\n");
		break;
	}

	if (*money <= 0) {
		printf("���� ��� ���������ϴ�. ������ �����մϴ�.\n");
		exit(0);
	}
	else new_round(table, player, dealer, money); //��������� ���ο� ���� ����
}

/*������ �ϸ��� �÷��̾�� ������ �ൿ ����
�ᱣ�� 1: �¸�, 2: ����, 3: ���� ����Ʈ, -1: �й�, -2: ���� ����, -3: ����Ʈ, 0: ���º�*/
int game_turn(card_stack* player, int* player_stay, card_stack* dealer, int* dealer_stay, card_stack* table) {

	show_game_state(*player, *dealer);
	if (get_card_sum(*player) == 21) return 2; //���� ����

	if (*player_stay == 0) {
		printf("h�� ���� ��Ʈ �ϰų� s�� ���� ���ĵ� �ϼ���.\n");
		while (1) {
			char temp_hs = getchar();
			if (temp_hs == 'h') {
				printf("��Ʈ�մϴ�.\n");
				while (getchar() != '\n'); //���� ���ڴ� ����
				put_card(player, pick_card_table(table));
				printf("���� ���� ī��: ");
				print_card(player->cards[player->card_count - 1]);
				printf("\n���� ī���� ��: %d\n\n", get_card_sum(*player));

				if (get_card_sum(*player) == 21) return 2; //����
				else if (get_card_sum(*player) > 21) return -3; //����Ʈ
				break;
			}
			else if (temp_hs == 's') {
				printf("���ĵ��߽��ϴ�. \n\n");
				while (getchar() != '\n'); //���� ���ڴ� ����
				*player_stay = 1;

				if (*dealer_stay) {
					printf("�÷��̾��� ī�� ��: %d, ������ ī�� ��: %d\n", get_card_sum(*player), get_card_sum(*dealer));
					if (get_card_sum(*player) > get_card_sum(*dealer)) return 1;
					else if (get_card_sum(*player) == get_card_sum(*dealer)) return 0;
					else return -1;
				}
				break;
			}
		}
	}
	if (*dealer_stay == 0) {
		if (get_card_sum(*dealer) == 21) return -2; //���� ����

		if (get_card_sum(*dealer) <= 16) {
			//��Ʈ
			printf("������ ��Ʈ �߽��ϴ�.\n");
			put_card(dealer, pick_card_table(table));
			printf("���� ���� ī��: ");
			print_card(dealer->cards[dealer->card_count - 1]);
			printf("\n\n");

			if (get_card_sum(*dealer) == 21) return -2; //���� ����
			else if (get_card_sum(*dealer) > 21) return 3; //���� ����Ʈ
		}
		else {
			//���ĵ�
			printf("������ ���ĵ� �߽��ϴ�.\n");
			*dealer_stay = 1;

			if (*player_stay) {
				printf("�÷��̾��� ī�� ��: %d, ������ ī�� ��: %d\n", get_card_sum(*player), get_card_sum(*dealer));
				if (get_card_sum(*player) > get_card_sum(*dealer)) return 1;
				else if (get_card_sum(*player) == get_card_sum(*dealer)) return 0;
				else return -1;
			}
		}
	}

	int result = game_turn(player, player_stay, dealer, dealer_stay, table); //������ ������� ���� ���, ��������� ���� �� ����
	return result;
}

//���� �÷��̾�� ������ ī�� ���¸� ������
void show_game_state(card_stack player, card_stack dealer) {
	printf("�÷��̾��� ī��: \n");
	for (int i = 0; i < player.card_count; i++) {
		print_card(player.cards[i]);
		printf(" ");
	}
	printf("\n�հ� : %d\n", get_card_sum(player));

	printf("������ ī��: \n");
	printf("?? "); //������ ī�� 1���� ������ ����
	for (int i = 1; i < dealer.card_count; i++) {
		print_card(dealer.cards[i]);
		printf(" ");
	}
	printf("\n\n");
}

//ī���� ���̿� ���ڸ� ���� �����
void print_card(card_type card) {
	char res[5];
	strcpy(res, card_to_string(card));
	printf("%s", res);
}

//���� ī�� ���ÿ� �ִ� ī�� ���� ��������� ��� �� ��ȯ��
int get_card_sum(card_stack deck) {
	int sum = 0, aces = 0;

	for (int i = 0; i < deck.card_count; i++) {
		if (deck.cards[i].number == 1) aces++; //���̽��� ���� ���
		else if (deck.cards[i].number > 10) sum += 10; //J, Q, K�� 10���� ���
		else sum += deck.cards[i].number;
	}

	for (; aces > 0; aces--) {
		if (aces == 1 && sum < 11) sum += 11; //������ ���̽������� ����������� ���� 10 ������ ��� �ش� ���̽��� 11�� ���
		else sum += 1; //�� ���� ��� 1�� ���
	}

	return sum;
}

// ī�� Ȯ�ο�, ī�� ������ ��� ī�带 �����
void see_all_cards(card_stack cards) {
	for (int i = 0; i < 52; i++) {
		print_card(pick_card(&cards));
		printf(" ");
		if ((i + 1) % 5 == 0) printf("\n");
	}
}