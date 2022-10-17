#include "betterC.h"

// Thank to https://stackoverflow.com/questions/24708700/c-detect-when-user-presses-arrow-key
// for arrow key detection
#define KEY_UP    72
#define KEY_DOWN  80
#define KEY_LEFT  75
#define KEY_RIGHT 77

#define ACTION_NULL  0
#define ACTION_RIGHT 1
#define ACTION_LEFT  2
#define ACTION_UP    3
#define ACTION_DOWN  4
#define ACTION_WAIT  5

#define action int
#define foreach_direction_and_null(_varName) for(action _varName = 0; _varName < 5; _varName++)
#define foreach_direction(_varName) for(action _varName = 1; _varName < 5; _varName++)
int directional_action(int x, int y)
{
    if(x != 0) { return x > 0 ? ACTION_RIGHT : ACTION_LEFT; }
    if(y != 0) { return y > 0 ? ACTION_UP    : ACTION_DOWN; }
    return ACTION_NULL;
}
bool action_is_directionnal(action a){ return a >= 1 && a <= 4; }



#define Id_Null          '?'
#define Id_Wall          '#'
#define Id_Air           '.'
#define Id_Player        'P'
#define Id_Follower      'F'
#define Id_Semi_Follower 'f'
#define Id_Ghost         'G'
#define Id_Semi_Ghost    'g'
#define Id_Gem           '$'

typedef struct
{
    char symbol;
    char* color;
}display;

typedef struct
{
    display;
    char id;
}things;

typedef struct
{
    things;
    int pos_x;
    int pos_y;
    int last_tick_update;
    action current_action;
}entity;

typedef struct
{
    char* name;
    char* path;

    things** block;
    int block_x;
    int block_y;

    entity* entities;
    int max_entity;

    bool gamerule_connected_edge_x;
    bool gamerule_connected_edge_y;

    int tick; 
}level;

bool is_solid(things t) { return t.id == Id_Wall; }

int entity_find_free_slot(level* l)
{
    repeat(i, l->max_entity)
    {
        if (l->entities[i].id == Id_Null)
        {
            return i;
        }
    }
    return -1;
}

void entity_die(entity* e)
{ 
    e->id = Id_Null;
}

things default_char_things(char c)
{
    return (things){.color = ANSI_COLOR_RESET, .symbol = c, .id = c };
}
things default_things()
{
    return default_char_things(Id_Null);
}

entity default_entity()
{
    return (entity){.color = null, .symbol = '?', .id = Id_Null, .pos_x = 0, .pos_y = 0, .last_tick_update = -1, .current_action = ACTION_NULL };
}
entity* spawn(things t, int x, int y, level* l)
{
    int slot = entity_find_free_slot(l);
    entity* e = null;
    if(slot != -1)
    {
        e = &l->entities[slot];
        *e = default_entity();
        *e = *((entity*)(&t));
        e->pos_x = x;
        e->pos_y = y;
        e->last_tick_update = l->tick;
    }
    return e;
}

level* level_from_file(char* path)
{
    file* f = fopen(path, "r");
    if(f == null){ return null; }

    level* l = new(level);
    if(l == null){ return null;}
    l->gamerule_connected_edge_x = false;
    l->gamerule_connected_edge_y = false;
    l->tick = 0;
    l->path = path;

    char c;
    int sizeX = 0;
    int currentX = 0;
    int sizeY = 1;

    l->name = new_array(char, 128);
    int k = 0;
    while (!feof(f))
    {
        c = ' ';
        fscanf(f, "%c", &c);

        switch(c)
        {
            case '\n': goto endName; break;
            case '-': l->gamerule_connected_edge_x = true; break;
            case '|': l->gamerule_connected_edge_y = true; break;
        }
        l->name[k++] = c;
    }
    endName:
    l->name[k++] = '\0';

    int levelDataOffset = ftell(f);

    while (!feof(f))
    {
        fscanf(f, "%c", &c);

        if (feof(f)) { break; }

        if (c == '\r' || c == '\n')
        {
            currentX = 0;
            sizeY++;
        }
        else
        {
            currentX++;
            sizeX = currentX > sizeX ? currentX : sizeX;
        }
    }
    fseek(f, levelDataOffset, SEEK_SET);

    l->block_x = sizeX;
    l->block_y = sizeY;

    l->block = new_array(things*, sizeX);
    repeat(i, sizeX) // enjoy my macros. - Thomas
    {
        l->block[i] = new_array(things, sizeY);
        repeat(j, sizeY)
        {
            l->block[i][j] = default_things();
        }
    }

    l->max_entity = 5;
    l->entities = new_array(entity, l->max_entity);
    repeat(i, l->max_entity)
    {
        l->entities[i] = default_entity();
    }

    int x = 0;
    int y = sizeY-1;
    fseek(f, levelDataOffset, SEEK_SET);

    // Reading level data
    while (!feof(f) && !(x == sizeX && y == 0))
    {
        c = ' ';
        fscanf(f, "%c", &c);
        
        if (c == '\r' || c == '\n' || feof(f))
        {
            y--;
            x = 0;
        }
        else
        {
            things* e = &(l->block[x][y]);
            *e = default_char_things(c);
            bool replaceByAir = false;
            
            switch (c)
            {
                case Id_Wall:
                    e->color = ANSI_COLOR_BLUE_BG;
                    e->symbol = '`';
                break;
                case Id_Player:
                {
                    things p = default_char_things(c);
                    p.color = ANSI_COLOR_YELLOW;
                    spawn(p, x, y, l);
                    replaceByAir = true;
                }break;
                case Id_Ghost:
                case Id_Semi_Ghost:
                case Id_Follower:
                case Id_Semi_Follower:
                {
                    things p = default_char_things(c); p.color = ANSI_COLOR_RED; spawn(p, x, y, l); replaceByAir = true;
                } break;
                case Id_Gem: e->color = ANSI_COLOR_GREEN; break;
                break;
            }

            if(replaceByAir)
            {
                *e = default_char_things(Id_Air);
            }
            x++;
        }
    }

    fclose(f);
    return l;
}

void print_things(things t)
{
    printf("%s%c%s", t.color ,t.symbol, ANSI_COLOR_RESET);
}

/*
int block_id_at_pos_equal(int x, int y, char id, level* l)
{
    if(inside_level(x, y, l))
    {
        return l->block[x][y].id == id ? 1 : 0;
    }
    return 0;
}*/

void print_block(int x, int y, level* l)
{
    things t = l->block[x][y];
    
    if(t.id == '#')
    {
        //int tileIdx = 1*block_id_at_pos_equal(x+1, y, '#', l) + 2*block_id_at_pos_equal(x-1, y, '#', l)+ 4*block_id_at_pos_equal(x, y+1, '#', l)+ 4*block_id_at_pos_equal(x, y-1, '#', l);
        //t.symbol = '+';
        //prin("%s", '▉');

    }
    print_things(t);
}

void print_level(level* l)
{
    printf("%s\n%i\n\n", l->name, l->tick);

    repeat_reverse(y, l->block_y)
    {
        repeat(x, l->block_x)
        {
            bool wasDisplay = false;
            repeat(e_idx, l->max_entity)
            {
                entity* e = &(l->entities[e_idx]);
                if(wasDisplay == false && e->id != Id_Null && e->pos_x == x && e->pos_y == y)
                {
                    print_things(*((things*)e));
                    wasDisplay = true;
                    break;
                }
            }
            if(wasDisplay == false)
            {
                print_block(x, y, l);
            }
        }
        printf("\n");
    }
    printf("\n\n\n");
//printf("\033[104m Hello  \033[0m");

    fflush(stdout);
}

action get_directional_action()
{
    char c;
    switch((c=getch()))
    {
        case KEY_UP   : return ACTION_UP;
        case KEY_DOWN : return ACTION_DOWN;
        case KEY_RIGHT: return ACTION_RIGHT;
        case KEY_LEFT : return ACTION_LEFT;
        case ' '      : return ACTION_WAIT;
            return c;
        default: return get_directional_action();
    }
}

bool inside_level(int x, int y, level* l) { return (uint32)x < (uint32)l->block_x && (uint32)y < (uint32)l->block_y; }

int action_to_direction_x(action dir) { return dir == ACTION_RIGHT ? 1 : (dir == ACTION_LEFT ? -1 : 0); }
int action_to_direction_y(action dir) { return dir == ACTION_UP    ? 1 : (dir == ACTION_DOWN ? -1 : 0); }

int entity_final_x_if_can_move(entity* e, action dir, level* l)
{
    int x = e->pos_x + action_to_direction_x(dir);
    if(inside_level(x, 0, l) == false && l->gamerule_connected_edge_x)
    {
        x = (x+l->block_x) % l->block_x;
    }
    return x;
}
int entity_final_y_if_can_move(entity* e, action dir, level* l)
{
    int y = e->pos_y + action_to_direction_y(dir);
    if(inside_level(0, y, l) == false && l->gamerule_connected_edge_y)
    {
        y = (y+l->block_y) % l->block_y;
    }
    return y;
}


bool entity_can_move(entity* e, action dir, level* l)
{
    if(action_is_directionnal(dir) == false){ return false;}

    int x = entity_final_x_if_can_move(e, dir, l);
    int y = entity_final_y_if_can_move(e, dir, l);

    if(inside_level(x, y, l) == false){ return false; }

    if(is_solid(l->block[x][y]))
    {
        return false;
    }
    return true;
}

bool entity_move(entity* e, action dir, level* l)
{
    if(entity_can_move(e, dir, l))
    {
        e->pos_x = entity_final_x_if_can_move(e, dir, l);
        e->pos_y = entity_final_y_if_can_move(e, dir, l);
    
        return true;
    }
    return false;
}

int length_squared_with_gamerule(int x1, int y1, int x2, int y2, level* l)
{
    int lx = abs(x1-x2);
    if(l->gamerule_connected_edge_x && lx > l->block_x /2){ lx = l->block_x - lx; }
    int ly = abs(y1-y2);
    if(l->gamerule_connected_edge_y && ly > l->block_y /2){ ly = l->block_y - ly; }
    return lx+ly;
}

void entity_attack_player(entity* e, level* l)
{
    repeat(i, l->max_entity)
    {
        if(l->entities[i].id == Id_Player)
        {
            if(l->entities[i].pos_x == e->pos_x && l->entities[i].pos_y == e->pos_y)
            {
                // player killed
                l->entities[i] = default_entity();
            }
        }
    }
}

void entity_follow_player(entity* e, level* l)
{
    int targetX = -1;
    int targetY = -1;

    repeat(i, l->max_entity)
    {
        if(l->entities[i].id == Id_Player)
        {
            targetX = l->entities[i].pos_x;
            targetY = l->entities[i].pos_y;
            break;
        }
    }

    if(targetX == -1){ return; }

    action best = ACTION_NULL;
    int length_after = 1000000;

    foreach_direction_and_null(current)
    {
        if(entity_can_move(e, current, l))
        {
            int px = entity_final_x_if_can_move(e, current, l);
            int py = entity_final_y_if_can_move(e, current, l);
            int current_length = length_squared_with_gamerule(targetX, targetY, px, py, l);

            if(current_length < length_after){ length_after = current_length; best = current;}
        }
    }
    
    entity_move(e, best, l);
}

void update_entity_follower(entity* e, level* l)
{
    entity_follow_player(e, l);
    entity_attack_player(e, l);
}

int count_player(level* l)
{
    int s = 0;
    repeat(e, l->max_entity)
    {
        if(l->entities[e].id == Id_Player){ s += 1;}
    }
    return s;
}

void swap_intptr(int** a, int** b)
{
    int* tmp = *b;
    *b = *a;
    *a = tmp;
}

action pathfinding_direction_to_players(entity* e, level *l)
{
    int oldPosX = e->pos_x;
    int oldPosY = e->pos_y;

    int nb_target =  count_player(l);
    int* target_x = new_array(int, nb_target); 
    int* target_y = new_array(int, nb_target);
    int targetIdx = 0;

    repeat(e, l->max_entity)
    {
        if(l->entities[e].id == Id_Player)
        {
            target_x[targetIdx] = l->entities[e].pos_x;
            target_y[targetIdx] = l->entities[e].pos_y;
            targetIdx++;
        }
    }

    
    int** path;
    path = new_array(int*, l->block_x);
    repeat(x, l->block_x)
    {
        path[x] = new_array(int, l->block_y);
        repeat(y, l->block_y)
        {
            path[x][y] = -1;
        }
    }

    int search_size = l->block_x + l->block_y + 1; //maybe less or more ?

    int* last_search_x = new_array(int, search_size); 
    int* last_search_y = new_array(int, search_size); 
    action* last_search_first_action = new_array(action, search_size);
    int last_search_idx = 1;

    int* next_search_x = new_array(int, search_size); 
    int* next_search_y = new_array(int, search_size);
    action* next_search_first_action = new_array(action, search_size);
    int next_search_idx;

    last_search_x[0] = e->pos_x;
    last_search_y[0] = e->pos_y;
    last_search_first_action[0] = ACTION_NULL;
    path[last_search_x[0]][last_search_y[0]] = 0;

    int turn = 0;

    action todo = ACTION_WAIT;
    
    while(last_search_idx != 0)
    {
        next_search_idx = 0;
        repeat(i, last_search_idx)
        {
            foreach_direction(dir)
            {
                e->pos_x = last_search_x[i];
                e->pos_y = last_search_y[i];

                if(entity_can_move(e, dir, l))
                {
                    entity_move(e, dir, l);
                    if(path[e->pos_x][e->pos_y] == -1)
                    {
                        path[e->pos_x][e->pos_y] = turn;
                        next_search_x[next_search_idx] = e->pos_x;
                        next_search_y[next_search_idx] = e->pos_y;
                        next_search_first_action[next_search_idx] = last_search_first_action[i] != ACTION_NULL ? last_search_first_action[i] :  dir;

                        repeat(t, nb_target)
                        {
                            if(target_x[t] == e->pos_x && target_y[t] == e->pos_y)
                            {
                                todo = next_search_first_action[next_search_idx];
                                goto end;
                            }
                        }
                        next_search_idx++;
                    }
                }
            }
        }

        last_search_idx = next_search_idx;
        swap_intptr(&next_search_x, &last_search_x);
        swap_intptr(&next_search_y, &last_search_y);
        swap_intptr(&last_search_first_action, &next_search_first_action);

        turn++;
    }
    end:
    
    sfree(target_x);
    sfree(target_y);

    sfree(last_search_x);
    sfree(last_search_y);
    sfree(last_search_first_action);

    sfree(next_search_x);
    sfree(next_search_y);
    sfree(next_search_first_action);
    
    repeat(x, l->block_x)
    {
        sfree(path[x]);
    }
    sfree(path);

    e->pos_x = oldPosX;
    e->pos_y = oldPosY;
    return todo;
}

bool entity_in_array(entity* e, entity** array, int arraySize)
{
    repeat(i, arraySize)
    {
        if(array[i] == e){ return true;}
    }
    return false;
}

void update_entity_ghost(entity* e_cur, level* l)
{
    #define is_target(_entity) entity_in_array(_entity, target, nb_target)
    entity_attack_player(e_cur, l);

    if(e_cur->current_action != ACTION_NULL)
    {
        entity_move(e_cur, e_cur->current_action != ACTION_WAIT ? e_cur->current_action : pathfinding_direction_to_players(e_cur, l), l);
        entity_attack_player(e_cur, l);
        return;
    }

    int nb_target =  count_player(l);
    entity** target = new_array(entity*, nb_target); 
    int targetIdx = 0;

    repeat(t, l->max_entity)
    {
        if(l->entities[t].id == Id_Player)
        {
            target[targetIdx] = &(l->entities[t]);
            targetIdx++;
        }
    }

    typedef struct
    {
        int x;
        int y;
        int first_action;
        entity* owner;
    }cell;

    typedef struct
    {
        entity* enemi;
        entity* target;
    }tile_stat;
    
    tile_stat** cellUsedBy;

    cellUsedBy = new_array(tile_stat*, l->block_x);
    repeat(x, l->block_x)
    {
        cellUsedBy[x] = new_array(tile_stat, l->block_y);
        repeat(y, l->block_y)
        {
            cellUsedBy[x][y] = (tile_stat){.enemi=null, .target=null};
        }
    }

    int nbGhost = 0;
    repeat(i, l->max_entity)
    {
        if(l->entities[i].id == Id_Ghost || l->entities[i].id == Id_Semi_Ghost)
        {
            nbGhost++;
        }
    }

    int search_size =  l->block_x * l->block_y; //(nbGhost+nb_target) *(l->block_x + l->block_y + 1); //maybe less or more ?
    cell* last_search = new_array(cell, search_size); 
    int last_search_idx = 0;

    repeat(i, l->max_entity)
    {
        entity* e = &(l->entities[i]);
        if(e->id == Id_Ghost || e->id == Id_Semi_Ghost)
        {
            e->current_action = ACTION_WAIT;
            cellUsedBy[e->pos_x][e->pos_y].enemi = e;
            last_search[last_search_idx++] = (cell){.x = e->pos_x, .y = e->pos_y, .first_action = e->current_action, .owner = e };
        }
    }
    repeat(i, l->max_entity)
    {
        entity* e = &(l->entities[i]);
        if(is_target(e))
        {
            e->current_action = ACTION_WAIT;
            cellUsedBy[e->pos_x][e->pos_y].target = e;
            last_search[last_search_idx++] = (cell){.x = e->pos_x, .y = e->pos_y, .first_action = e->current_action, .owner = e };
        }
    }


    cell* next_search = new_array(cell, search_size); 
    int next_search_idx;

    int turn = 0;
    
    until(last_search_idx <= 0 || nbGhost <= 0)
    {
        next_search_idx = 0;
        repeat(i, last_search_idx)
        {
            entity* e = last_search[i].owner;
            

            if(e == null){ continue;}
            bool isTarget = is_target(e);
            if(isTarget && e->current_action != ACTION_WAIT){ continue;}

            bool stopSearching4ThisEntity = false;

            int oldPosX = e->pos_x;
            int oldPosY = e->pos_y;

            if(e->id == Id_Semi_Ghost && ((l->tick+turn) % 2 == 0))
            {
                next_search[next_search_idx++] = (cell){ .x=last_search[i].x, .y=last_search[i].y, .owner = last_search[i].owner, .first_action = last_search[i].first_action  };
            }else foreach_direction(dir)
            {
                if(stopSearching4ThisEntity){ break;}

                e->pos_x = last_search[i].x;
                e->pos_y = last_search[i].y;

                if(entity_move(e, dir, l))
                {
                    
                    bool continueSearchingLocalPath  = true;
                    bool continueSearchingEntityPath = true;

                    if(cellUsedBy[e->pos_x][e->pos_y].enemi == null && cellUsedBy[e->pos_x][e->pos_y].target == null)
                    {
                        if(isTarget)
                        {
                            cellUsedBy[e->pos_x][e->pos_y].target = e;
                        }else
                        {
                            cellUsedBy[e->pos_x][e->pos_y].enemi = e;
                        }
                    }else 
                    {
                        continueSearchingLocalPath = false;
                        /*
                        if(isTarget)
                        {
                            e->current_action = last_search[i].first_action;
                        }*/
                        if(!isTarget && e->current_action == ACTION_WAIT && cellUsedBy[e->pos_x][e->pos_y].target != null && cellUsedBy[e->pos_x][e->pos_y].enemi == null)
                        {
                            continueSearchingEntityPath = false;
                            e->current_action = last_search[i].first_action;
                            nbGhost--;
                        }
                    }

                    if(continueSearchingLocalPath && (isTarget || e->current_action == ACTION_WAIT))
                    {
                        next_search[next_search_idx++] = (cell){ .x=e->pos_x, .y=e->pos_y, .owner = last_search[i].owner, .first_action = (last_search[i].first_action != ACTION_WAIT ? last_search[i].first_action : dir)  };
                    }

                    if(continueSearchingEntityPath == false)
                    {
                        stopSearching4ThisEntity = true;

                        repeat(j, next_search_idx)
                        {
                            if(next_search[j].owner == e)
                            {
                                next_search[j].owner = null;
                            }
                        }
                    }
                    
                }
                
                e->pos_x = oldPosX;
                e->pos_y = oldPosY;
            }
        }

        last_search_idx = next_search_idx;

        cell* tmp = last_search;
        last_search = next_search;
        next_search = tmp;
        turn++;
    }
    
    sfree(target);

    sfree(last_search);
    sfree(next_search);
    
    repeat(x, l->block_x)
    {
        sfree(cellUsedBy[x]);
    }
    sfree(cellUsedBy);
    update_entity_ghost(e_cur, l);
}

void update_entity_semi_follower(entity* e, level* l)
{
    entity_attack_player(e, l);
    if(l->tick % 2)
    {
        e->color = ANSI_COLOR_WHITE;
        entity_follow_player(e, l);
        entity_attack_player(e, l);
    }else
    {
        e->color = ANSI_COLOR_RED;
    }
}

void update_entity_semi_ghost(entity* e, level* l)
{
    if(l->tick % 2)
    {
        e->color = ANSI_COLOR_WHITE;
        update_entity_ghost(e, l);
    }else
    {
        e->color = ANSI_COLOR_RED;
        entity_attack_player(e, l);
    }
}

void update_entity_player(entity* e, level* l)
{
    action dir;
    do
    {
        dir = get_directional_action(); 
    }until(entity_can_move(e, dir, l) || dir == ACTION_WAIT);

    entity_move(e, dir, l);
    e->current_action = dir;

    if(l->block[e->pos_x][e->pos_y].id == '$')
    {
        l->block[e->pos_x][e->pos_y] = default_char_things(Id_Air);
    }
}

void entity_was_updated(entity* e, level* l)
{
    e->last_tick_update = l->tick;
}
void level_update(level* l)
{
    l->tick++;
    repeat(i, l->max_entity)
    {
        l->entities[i].current_action = ACTION_NULL;
    }

    repeat(i, l->max_entity)
    {
        entity* e = &(l->entities[i]);
        
        if(e->id == Id_Player) 
        { 
            update_entity_player(e, l);
            entity_was_updated(e, l); 
        }
    }

    repeat(i, l->max_entity)
    {
        entity* e = &(l->entities[i]);

        switch (e->id)
        {
            case Id_Follower:      update_entity_follower(e, l); break;
            case Id_Semi_Follower: update_entity_semi_follower(e, l); break;
            case Id_Ghost:         update_entity_ghost(e, l); break;
            case Id_Semi_Ghost:    update_entity_semi_ghost(e, l); break;
            default: break;
        }
        entity_was_updated(e, l);
    }
}

bool level_is_game_over(level* l)
{
    return count_player(l) == 0;
}

void level_run(level* l)
{
    if(l == null) { printf("level was null"); return; }

    goto middle;
    while(!level_is_game_over(l))
    {
        level_update(l);
        middle:
        print_level(l);
    }
    printf("Game Over !\n\n\n");
}

void level_unload(level* l)
{
    if(l == null){return;}
    repeat(x, l->block_x)
    {
        sfree(l->block[x]);
    }
    sfree(l->block);
    sfree(l->name);
    sfree(l->entities);
    sfree(l);
}

level* reset_level(level* l)
{
    level* afterReset = level_from_file(l->path);
    level_unload(l);
    return afterReset;
}


level* game_run(level* l)
{
    forever
    {
        level_run(l);
        l = reset_level(l);
    }
    return l;
}

int main()
{
    printf("Compilation of %s\n", __TIME__);

    level* l = level_from_file("./bin/content/l1.txt"); 
    if(l == null)
    {
        l = level_from_file("./content/l1.txt");
    }
    l = game_run(l);
    level_unload(l);

    printf("\nEnd\n");
    return EXIT_SUCCESS;
}

// cd bin
// gcc ../src/main.c -o main -Wall -Wextra && ./main.exe