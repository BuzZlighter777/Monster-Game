#include <iostream>
#include <string>
#include <cstdlib> 
#include <ctime> 

static bool gMarkMet{};

int getRandomNumber(int min, int max)
{
	static const double fraction = 1.0 / (static_cast<double>(RAND_MAX) + 1.0);
	return static_cast<int>(rand() * fraction * (max - min + 1) + min);
}

class Creature
{
protected:
	std::string m_name;
	char m_symbol;
	int m_hp;
	int m_dmg;
	int m_gold;

public:
	Creature(std::string name, char symbol, int hp, int dmg, int gold) : m_name{ name }, m_symbol{ symbol }, m_hp{ hp }, m_dmg{ dmg }, m_gold{ gold }
	{}

	Creature() {} //I added this to provide necessary Mark`s call behaviour (explained below)

	std::string getName() const { return m_name; }
	char getSymbol() const { return m_symbol; }
	int getHp() const { return m_hp; }
	int getDmg() const { return m_dmg; }
	int getGold() const { return m_gold; }

	void reduceHealth(int dmg)
	{
		m_hp -= dmg;
	}

	bool isDead()
	{
		return m_hp <= 0;
	}

	void addGold(int gold)
	{
		std::cout << "You found " << gold << " gold.\n";
		m_gold += gold;
	}

	void reduceGold(int gold) //for Mark`s bribery (look below)
	{
		m_gold -= gold;
	}
};

class Player : public Creature
{
	int m_lvl = 1;

public:
	Player(std::string name) : Creature(name, '@', 10, 1, 0) 
	{}

	void lvlUp()
	{
		std::cout << "Level up!\n";
		++m_lvl;
		++m_dmg;
	}

	void heal(int healCoefficient)
	{
		if (getHp() == 10) return; //you cannot heal if your hp is max

		if (getRandomNumber(1, 100) <= healCoefficient) //the larger is healCoefficient (monster`s property, look below), the larger is the chance to heal
		{
			if (getHp() == 9) //to prevent healing over max health
			{
				++m_hp;
				std::cout << "You also healed for 1hp!\n";
			}
			else
			{
				m_hp += 2;
				std::cout << "You also healed for 2hp!\n";
			}
		}
	}

	int getLvl() const { return m_lvl; }

	bool hasWon() { return m_lvl >= 20; }

	void getStatus() //print all main paramaters 
	{
		std::cout << "\nYou have " << getHp() << " health and are carrying " << getGold() << " gold.\n"
			<< "Your current level: " << getLvl() << '\n' << '\n';
	}
};

class Monster : public Creature
{
	int m_healCoefficient; //the actual probability to be healed after killing certain type of a monster

public:
	bool m_markBribery{}; //only for Mark`s scenario (didn`t know how to do otherwise)

	enum Type
	{
		DRAGON,
		ORC,
		SLIME,
		TROLL,
		MINDLORD,
		BLEDEN_MARK,
		MAX_TYPES
	};

	struct MonsterData
	{
		std::string name;
		char symbol;
		int hp;
		int dmg;
		int gold;
		int healCoefficient;
	};

	static MonsterData monsterData[MAX_TYPES];

	static Monster getRandomMonster()
	{
		return Monster(static_cast<Type>(getRandomNumber(0, MAX_TYPES - 1)));
	}

	Monster(Type type) : Creature(monsterData[type].name, monsterData[type].symbol, monsterData[type].hp, monsterData[type].dmg, monsterData[type].gold),
		m_healCoefficient{ monsterData[type].healCoefficient }
	{}

	Monster() {} //I added this to provide necessary Mark`s call behaviour (explained below)

	int getHealCoefficient() const { return m_healCoefficient; }
};

Monster::MonsterData Monster::monsterData[Monster::MAX_TYPES]
{
	{ "dragon", 'D', 10, 4, 100, 75}, //provides high chance to heal
	{ "orc", 'o', 4, 2, 25, 30 },
	{ "slime", 's', 1, 1, 10, 10 }, //provides very poor chance to heal
	{ "troll", 't', 5, 3, 40, 50}, //new: this early-game unit makes up for dragon`s "nerf"
	{ "Mindlord", 'M', 15, 5, 300, 100}, //new: guarantees healing, but you cannot espace from the Mindlord! (implemented below)
	{ "Bleden Mark", 'B', 9999, 100, 500, 0 } //new: cannot be killed, one-shots player, has low chance of letting you go, but you can bribe him if you have 1000+ gold! (implemented below)
};

bool wannaFight(Player& player, Monster& monster)
{
	char c;
	if (monster.getSymbol() == 'B') //implement Mark`s bribery scenario
	{
		std::cout << "\nBleden Mark: Shall I put your out of your misery?\n";
		do
		{
			std::cout << "(R)un or (F)ight, or (B)ribe (1000 gold) ";
			std::cin >> c;
		} while (c != 'r' && c != 'R' && c != 'f' && c != 'F' && c != 'b' && c != 'B');

		if (c == 'f' || c == 'F') return true;
		else if (c == 'r' || c == 'R')return false;
		else
		{
			std::cout << "You: Please, don`t kill me! I`ll pay you 1000 gold.\n";

			if (player.getGold() >= 1000)
			{
				std::cout << "Bleden Mark: Hhhm, that`s wise of you.\n";
				player.reduceGold(1000);
				std::cout << "Bleden Mark: Pray you don`t face me again...\n";
				monster.m_markBribery = true;
				return false;
			}
			else
			{
				std::cout << "Bleden Mark: Are you trying to fool me?! I know you don`t have 1000 gold!\n";
				return true; //we can assume Mark attacks the player here
			}
		}
	}

	//all other cases
	do
	{
		std::cout << "(R)un or (F)ight: ";
		std::cin >> c;
	} while (c != 'r' && c != 'R' && c != 'f' && c != 'F');

	if (c == 'f' || c == 'F') return true;
	else return false;
}

bool fled(int level, char monster)
{
	if (monster == 'M') return false; //implement "escape denied" scenario with Mindlord
	if (monster == 'B') return getRandomNumber(1, 10) > 8; //fat chance that Bleden Mark gonna lose you! (20%)

	//and these are cases with standard monsters
	if (level > 5 && level < 10)
		return getRandomNumber(1, 10) <= 6; // +10% from the base value
	else if (level >= 10)
		return getRandomNumber(1, 10) <= 7; // +20% from the base value
	else
		return getRandomNumber(0, 1) == 1; //base value (50%)
}

void fight(Player& player, Monster& monster)
{
	while (!player.isDead())
	{
		monster.reduceHealth(player.getDmg());
		std::cout << "You hit the " << monster.getName() << " for " << player.getDmg() << " damage.\n";

		if (monster.isDead())
		{
			std::cout << "You killed the " << monster.getName() << "!\n";
			player.addGold(monster.getGold());
			player.lvlUp();
			player.heal(monster.getHealCoefficient()); //healing done here (which may or may not succeed)

			return;
		}

		player.reduceHealth(monster.getDmg());
		std::cout << "The " << monster.getName() << " hit you for " << monster.getDmg() << " damage.\n";
	}
}

void initiating(Player& player, Monster& monster)
{
	std::cout << "You have encountered a " << monster.getName() << " (" << monster.getSymbol() << ").\n";
	if (monster.getSymbol() == 'M') std::cout << "Don`t try to run from him!\n";    //these are hints for special
	if (monster.getSymbol() == 'B') std::cout << "Don`t even try to attack him!\n"; //high-level monsters

	if (!wannaFight(player, monster)) //passing both player and monster to implement Bleden Mark`s scenario
	{
		if (monster.m_markBribery) return; //when Mark`s bribery went successfully

		while (!fled(player.getLvl(), monster.getSymbol()) && !player.isDead()) //passing both player and monster to regulate the escape`s chance
		{
			player.reduceHealth(monster.getDmg());

			std::cout << "\nYou failed to flee.\n" << '\n'
				<< "The " << monster.getName() << " hit you for " << monster.getDmg() << " damage.\n";

			if (player.isDead()) return;
			if (!wannaFight(player, monster)) continue; //if player`s choice is the same (try to flee), repeat
			else
				goto PlayerChangedHisMind; //if player changes his choice, start fighting
		}
		std::cout << "\nYou successfully fled.\n";
		return;
	}

PlayerChangedHisMind: //how to do this without goto???
	fight(player, monster);
}

bool appropriateEnemy(int lvl, char monster) //this excludes cases when player confronts high-level monsters, being low-level himself
{
	if ((monster == 'D' || monster == 'M' || monster == 'B') && lvl < 5) return false;
	if ((monster == 'M' || monster == 'B') && lvl < 10) return false;
	if (monster == 'B' && (lvl < 15 || gMarkMet)) return false;

	return true;
}


int main()
{
	srand(static_cast<unsigned int>(time(0)));
	rand();

	std::string userName;
	std::cout << "Enter your name: ";
	std::cin >> userName;
	Player player(userName);

	std::cin.ignore(32767, '\n');

	std::cout << "Welcome, " << userName << '\n';

	bool MarkFirstMet{};
	while (!player.isDead() && !player.hasWon())
	{
		
		player.getStatus();

		while (true)
		{
			Monster monster;
			if (!MarkFirstMet && player.getLvl() == 19) monster = Monster::BLEDEN_MARK; //this is done to ensure Mark is called at least once, at the end of the game
			else monster = Monster::getRandomMonster();                               //the whole idea here is not to let player win, if he/she`d farm slimes & orcs all the time, avoiding "big guys"
																				   //that way he/she would have no money to bribe Mark

			if (appropriateEnemy(player.getLvl(), monster.getSymbol())) //check if the monster is relevant for our player`s level
			{
				initiating(player, monster);
				if (monster.getSymbol() == 'B')
				{
					gMarkMet = true;
					MarkFirstMet = true;
				}
				break;
			}
		}
	}

	if (player.hasWon())
		std::cout << "\nYou won with " << player.getGold() << " gold!\n";

	if (player.isDead())
		std::cout << "\nYou died at level " << player.getLvl() << " with " << player.getGold() << " gold.\n"
		<< "Too bad you can't take it with you!";

	return 0;
}