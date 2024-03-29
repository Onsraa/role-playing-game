#include "../Struct/struct.h"

enum action
{
    BASIC_ATTACK = 1,
    OFFENSIVE = 2,
    HEAL = 3
};

enum gearState
{
    EQUIPPED = 1,
    STORED = 0
};

void gainXp(Character *character, Mob *mob){

    int xp_gained = XP_GAINED * mob->level;

    printf("You gained %d XP\n\n", xp_gained);

    character->experience += xp_gained;

    if(character->experience >= character->experienceNeededToLevelUp){
        levelUp(character);
    }
}

void levelUp(Character *character){

    character->experience = 0;
    character->experienceNeededToLevelUp *= 1.5;
    character->level++;

    character->physicalPower *= 1.5;
    character->magicalPower *= 1.5;
    
    character->maxHp *= 1.3;
    character->maxMp *= 1.3;

    updateSpells(character);
    resetCharacter(character);

    printf("You leveled up ! You are now level %d\n\n", character->level);
}

void fight(Character *character, Mob *mob, int auto_mode, int dialogue)
{
    int rounds = 0;

    system("clear");
    puts(" ");
    printf("##########################################");
    puts(" ");
    printf("\tFight: %s VS %s", character->className, mob->name);
    puts(" ");
    printf("##########################################");
    puts(" ");

    switch (auto_mode)
    {
    case 0:
        break;
    case 1:
        while (character->isAlive && mob->isAlive)
        {   
            puts(" ");
            printf("\tROUND N°%d\n\n", rounds);
            switch (rounds % 2)
            {
            case 0:
                players_turn(character, mob, dialogue);
                break;
            case 1:
                mobs_turn(mob, character, dialogue);
                break;
            }
            showFightStates(character, mob);
            regenerateMana(character);
            printf("--------------------------------------------------------------\n");
            ++rounds;
        }
        break;
    }

    if(character->isAlive){
        printf("You won against the %s !", mob->name);
        gainXp(character, mob);
        dropStuff(character, mob);
        printf("\n\n1 - Continue\n\n");
    }else{
        printf("You died against the %s.\n\n", mob->name);
        printf("\n\n1 - Go back to menu\n\n");
    }
    puts(" ");

    int choice;

    do{
        scanf("%d", &choice);
    }while(choice != 1);

}

void players_turn(Character *character, Mob *mob, int dialogue)
{

    if (!character || !mob)
    {
        system("clear");
        printf("Character or mob don't exist.");
        exit(EXIT_FAILURE);
    }

    int total_damage = character->physicalPower + character->magicalPower;

    if (character->gears && character->gears->weapon)
    { // Check if a weapon is equipped.
        switch (compability(character->element, character->gears->weapon->element))
        { // Check element compability between weapon and character.

        case 1: // Character's element is efficient against the weapon so it changes nothing.
            total_damage += character->gears->weapon->bonus_damage;
            break;
        case 2: // Weapon's element is efficient against the character so it gives mallus.
            if (dialogue)
            {
                printf("The character struggles to handle a weapon of the opposite element : mallus !\n");
            }
            total_damage += character->gears->weapon->bonus_damage * 0.7;
            break;
        case 3: // Same element weapon and character so bingo.
            if (dialogue)
            {
                printf("Compability between weapon and character : bonus damage !\n");
            }
            total_damage += character->gears->weapon->bonus_damage * 2;
            break;
        }
    }

    Spell *offensive_spell = character->spells[0];
    Spell *healing_spell = character->spells[1];

    switch (fightAlgorithm(character, mob))
    {
    case 1: // BASIC ATTACK
        mob->currentHp -= total_damage;

        if (dialogue)
        {
            printf("You hit the %s for %d with your basic attack !\n", mob->name, total_damage);
        }

        break;
    case 2: // OFFENSIVE ATTACK

        character->currentMp -= character->spells[0]->cost;

        if(character->currentMp < 0){
           character->currentMp = 0; 
        }

        if (character->spells[0]->valueFactor == STATIC)
        {
            total_damage += offensive_spell->value;
        }
        else
        {
            total_damage *= offensive_spell->value;
        }

        mob->currentHp -= total_damage;

        if (dialogue)
        {
            printf("You used your offensive spell ! The %s, it hits for %d !\n", offensive_spell->spellName, total_damage);
        }

        break;
    case 3: // HEALING

        character->currentMp -= character->spells[1]->cost;

        if(character->currentMp < 0){
           character->currentMp = 0; 
        }

        if (character->spells[1]->valueFactor == STATIC)
        {
            total_damage += healing_spell->value;
        }
        else
        {
            total_damage *= healing_spell->value;
        }

        character->currentHp += total_damage;

        if (dialogue)
        {
            printf("You healed yourself for %d with the %s !\n\n", total_damage, healing_spell->spellName);
        }
        break;
    }

    if(mob->currentHp <= 0){
        mob->isAlive = 0;
    }
}

void mobs_turn(Mob *mob, Character *character, int dialogue)
{

    if (!character || !mob)
    {
        system("clear");
        printf("Character or mob don't exist.");
        exit(EXIT_FAILURE);
    }

    int total_damage = mob->physicalPower + mob->magicalPower;

    if (dialogue)
    {
        printf("The %s hits you for %d !\n", mob->name, total_damage, character->currentHp);
    }
    character->currentHp -= total_damage;

    if(character->currentHp <= 0){
        character->isAlive = 0;
    }
}

int fightAlgorithm(Character *character, Mob *mob)
{

    if (!character)
    {
        printf("No character.");
        exit(EXIT_FAILURE);
    }

    int isLow = (character->currentHp <= character->maxHp * 0.3) ? 1 : 0; // State to know if the character is low in hp or not (>30% hp).

    int basic_attack_damage = character->physicalPower + character->magicalPower;

    if (basic_attack_damage >= mob->currentHp)
    { // If the character only needs to hit once to kill the mob, then it returns the action.
        return BASIC_ATTACK;
    }

    Spell *offensive_spell = character->spells[0];
    Spell *healing_spell = character->spells[1];

    switch (isLow)
    {
    case 0:
        if (character->currentMp >= offensive_spell->cost)
        { // Check if we have enough mana to cast an offensive.
            return OFFENSIVE;
        }
        break;
    case 1:
        if (character->currentMp >= healing_spell->cost)
        { // Check if we have enough mana to cast a heal.
            return HEAL;
        }
        break;
    }

    return BASIC_ATTACK;
}

void showFightStates(Character *character, Mob *mob){

    printf("You :\n");
    showBars(character);
    puts(" ");
    printf("The %s : \n", mob->name);

    int currentHpBar = mob->currentHp * BAR_LENGTH / mob->maxHp;
    
    printf("HP (%d/%d) : ", mob->currentHp, mob->maxHp);
    printf("[");
    for(int i = 0 ; i < currentHpBar ; i++){
        printf("#");
    }
    for(int i = currentHpBar ; i < BAR_LENGTH ; i++){
        printf(" ");
    }
    printf("]");
    puts(" ");
}

void regenerateMana(Character * character){

    character->currentMp *= MANA_REGEN_RATIO;

    if(character->currentMp > character->maxMp){
        character->currentMp = character->maxMp;
    }
}

