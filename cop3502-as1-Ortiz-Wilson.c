#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "sp21_cop3502_as1.h"
#include "leak_detector_c.h"

void remove_crlf(char *s);
int get_next_nonblank_line(FILE *ifp, char *buf, int max_length);


//monster struct functions
int get_num_monsters(FILE *ifp);
void monster_constructor(FILE *ifp, int num_monsters, monster *m);
void read_monster(FILE *ifp, monster *m);
void monster_destructor(monster *m, int num_monsters);

//region struct functions
int get_num_regions(FILE *ifp);
void region_constructor(FILE *ifp, int num_regions, int num_monsters, region *r, monster *m);
void read_region(FILE *ifp, region *r, int num_monsters, monster *m);
void region_destructor(region *r, int num_regions);

//trainer struct functions
int get_num_trainers(FILE *ifp);
void trainer_constructor(FILE* ifp, int num_trainers, trainer *t, region *r, itinerary *itin, int region_num_regions);
void read_trainer(FILE *ifp, trainer *t, region *r, itinerary *itin, int region_num_regions);

void itinerary_constructor(FILE* ifp, int num_captures, int trainer_num_regions, trainer *t, region *r, itinerary *itin, int region_num_regions);
void itinerary_destructor(itinerary *itin, int num_trainers);

void calculate_captures(FILE *ofp, trainer *t, int region_index);
void output_to_file(FILE *ofp, monster *m, region *r, itinerary *itin, trainer *t, int num_trainers);



//function provided by Professor Gerber in class
void remove_crlf(char *s)
{
    /* Remember that in C, a string is an array of characters ending with a '\0' character.  We
       are given s, a pointer to this array.  We want to find the end of the string that s points
       to.  We know that strlen() will give us the *length* of s, so we add s's length to its
       location to find the end of s. */

    char *t = s + strlen(s);

    /* t is now at s's null terminator. */

    t--;

    /* t now points to the last character of s.  (Importantly, we haven't changed s.) */

    /* \n is the UNIX-like newline character.  MS-DOS and Windows occasionally add \r, which is a
       separate "carriage return" character.  We need to find and clobber both. */

    /* We have two cases where t can end up to the left of s: if we're passed a zero-length string,
       or if literally the entire string is made up of \n and/or \r.  Stop everything if t ends up
       to the left of s to prevent bad things from happening in that case. */

    /* Look at the contents of t as a single character.  As long as it's still a newline or a CR... */

    while((t >= s) && (*t == '\n' || *t == '\r'))
    {
        *t = '\0';  // ...clobber it by overwriting it with null, and...
        t--;        // decrement t *itself (i.e., change the pointed location)

        /* t now still points to the last character of s, and we keep going until it's something
           other than a CR or newline. */
    }
}

int get_next_nonblank_line(FILE *ifp, char *buf, int max_length)
{
    buf[0] = '\0';

    while(!feof(ifp) && (buf[0] == '\0'))
    {
        fgets(buf, max_length, ifp);
        remove_crlf(buf);
    }

    if(buf[0] != '\0') 
    {
        return 1;
    } 
    else 
    {
        return 0;
    }
}

//file gets passed in to grab the number of monsters
int get_num_monsters(FILE *ifp)
{
    char buf[256];
    int nmonsters;

    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d monsters", &nmonsters);

    return nmonsters;
}

//file and monster pointer get passed in and information is assigned to that specific monster
void read_monster(FILE *ifp, monster *m)
{
    int i;
    char buf[256];
    char name[40];
    char element[40];
    int population;

    //gets the next line and parses it for relavant information
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%s %s %d", name, element, &population);

    //assigns information parsed into the monster struct
    m->name = strdup(name);
    m->element = strdup(element);
    m->population = population;
}

//function to create a new monster
void monster_constructor(FILE *ifp, int num_monsters, monster *m)
{
    int i;

    //loops num_monster times to create that many monster structs
    for(i = 0; i < num_monsters; i++)
    {
        read_monster(ifp, &(m[i]));
    }
}

//file is passed in to grab the number of regions
int get_num_regions(FILE *ifp)
{
    int num_regions;
    char buf[256];

    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d regions", &num_regions);

    return num_regions;
}

//function to create new region
void region_constructor(FILE *ifp, int num_regions, int num_monsters, region *r, monster *m)
{
    int i;

    //loops num_regions times to create each region
    for(i = 0; i < num_regions; i++)
    {
        read_region(ifp, &(r[i]), num_monsters, m);
    }
}

//function that assigns appropriate information to region struct
void read_region(FILE *ifp, region *r, int num_monsters, monster *m)
{
    int i;
    int j;
    char buf[256];
    char name[40];
    int nmonsters;
    int total_population;

    //parses lines to get information for regions name and number of monster types
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%s", name);
    r->name = strdup(name);
    printf("Region name: %s\n", r->name);
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d monsters", &nmonsters);
    r->nmonsters = nmonsters;

    //allocates space for the pointer to pointer of monsters in the region struct
    r->monsters = malloc(sizeof(monster *) * nmonsters);

    // outer loop loops through the number of monster types in the region struct and grabs their name
    for(i = 0; i < nmonsters; i++)
    {
        get_next_nonblank_line(ifp, buf, 255);
        sscanf(buf, "%s", name);

        //inner loop loops through the number of monster structs and compares the name of the monster struct to the name grabbed
        for(j = 0; j < num_monsters; j++)
        {
            //if the names match, the regions monster will point to that monster struct and add to the regions total population
            if(strcmp(m[j].name, name) == 0)
            {
                r->monsters[i] = &(m[j]);
                r->total_population += m[j].population;
            }
        }
    }
    
}

//gets the number of trainers
int get_num_trainers(FILE *ifp)
{
    int num_trainers;
    char buf[256];

    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d trainers", &num_trainers);

    return num_trainers;
}

//loops through num_trainers times to create all the trainer structs
//we pass in a pointer to trainer struct to be incremented each loop as well as itinerary, this way we are referring to the next trainer and itinerary each loop
//region_num_regions is needed for a later function which is called in here
void trainer_constructor(FILE* ifp, int num_trainers, trainer *t, region *r, itinerary *itin, int region_num_regions)
{
    int i;

    for(i = 0; i < num_trainers; i++)
    {
        //we call read_trainer to get the rest of the necessary info for trainer as well as itinerary from the input file
        read_trainer(ifp, t + i, r, itin + i, region_num_regions);
    }
}

//function to grab info from the file and put it into the trainer struct and itinerary struct
void read_trainer(FILE *ifp, trainer *t, region *r, itinerary *itin, int region_num_regions)
{
    char buf[256];
    char name [40];
    int num_captures;
    int trainer_num_regions;

    //here we grab the name of the trainer and assign it
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%s", name);
    t->name = strdup(name);
    printf("trainer name: %s\n", t->name);

    //here we grab nregions and captures for the itinerary struct
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d captures", &num_captures);
    get_next_nonblank_line(ifp, buf, 255);
    sscanf(buf, "%d regions", &trainer_num_regions);

    //we then call the itinerary constructor and pass in necessary information
    itinerary_constructor(ifp, num_captures, trainer_num_regions, t, r, itin, region_num_regions);


}

//function to create itineraries
void itinerary_constructor(FILE* ifp, int num_captures, int trainer_num_regions, trainer *t, region *r, itinerary *itin, int region_num_regions)
{
    int i;
    int j;
    char buf[256];
    char region[40];

    printf("trainer_num_regions: %d\n", trainer_num_regions);
   
   //malloc the number of region pointers needed for the specific itinerary according to the info grabbed from input file
   itin->regions = malloc(sizeof(*region) * trainer_num_regions);

    //here we loop through based on how many regions we must read from the file
    for(i = 0; i < trainer_num_regions; i++)
    {
        //here we grab the name of the region in the file
        get_next_nonblank_line(ifp, buf, 255);
        sscanf(buf, "%s", region);

        //we then loop through all the region structs
        for(j = 0; j < region_num_regions; j++)
        {
            //we use this if statement to compare the region name grabbed from the file to the name located in the region struct
            //once it finds the correct struct we can assign the itineraries **region with the appropriate region
            if(strcmp(region, r[j].name) == 0)
            {
                itin->regions[i] = &(r[j]);
                itin->nregions = trainer_num_regions;
                itin->captures = num_captures;
                t->visits = itin;
                printf("trainer 1 itinerary: %s\n", itin->regions[i]->name);
                printf("t->visits: %d\n", t->visits->nregions);
            }
        }
    }
}

//function that handles outputting to the file
void output_to_file(FILE *ofp, monster *m, region *r, itinerary *itin, trainer *t, int num_trainers)
{
    int i;
    int j;

    //to get the appropriately formatted output we must loop through each trainers 
    //witihin that loop we must loop through each trainers region visited
    //within that loop we must call caculate_captures to determine the probability of capturing a monster in said region
    for(i = 0; i < num_trainers; i++)
    {
        fprintf(ofp, "%s\n", t[i].name);
        printf("trainer num regions: %d\n", t[i].visits->nregions);
        
        for(j = 0; j < t[i].visits->nregions; j++)
        {
            fprintf(ofp, "%s\n", t[i].visits->regions[j]->name);
            calculate_captures(ofp, &(t[i]), j);
        }
        fprintf(ofp,"\n");
    }
}

//function that calculates the expected number of captures in a given region
//the indexed trainer gets passed in from output_to_file so t refers to the i'th trainer 
//we pass in the region_index from the j loop in output_to_file so we can refer to the correct region within the trainers index
void calculate_captures(FILE *ofp, trainer *t, int region_index)
{
    int i;
    int num_captures;
    float monster_capture_ratio;
    float expected_num_monster;

    num_captures = t->visits->captures;
    
    //we loop through the number of monsters in the region to calculate each ones capture ratio
    //after obtaining the capture ratio we multiply it by the number of captures the trainer has to get the expected number of captures 
    for(i = 0; i < t->visits->regions[region_index]->nmonsters; i++)
    {
       monster_capture_ratio = (t->visits->regions[region_index]->monsters[i]->population) / (t->visits->regions[region_index]->total_population * 1.0);
       expected_num_monster = round(monster_capture_ratio * num_captures);

       if(expected_num_monster > 0)
       {
        fprintf(ofp, "%d %s \n", (int)expected_num_monster, t->visits->regions[region_index]->monsters[i]->name);
       }
    }

}

//frees monster struct
void monster_destructor(monster *m, int num_monsters)
{
    int i;

    //loops through each monster struct and frees its corresponding name and element
    for(i = 0; i < num_monsters; i++)
    {
        free(m[i].name);
        free(m[i].element);
    }

    //frees the array of monster structs
    free(m);
}

//frees region struct
void region_destructor(region *r, int num_regions)
{
    int i;

    //loops through each region and frees its corresponding name and monster array
    for(i = 0; i < num_regions; i++)
    {
        free(r[i].name);
        free(r[i].monsters);
    }

    //frees the array of region structs
    free(r);
}

//frees itinerary struct
void itinerary_destructor(itinerary *itin, int num_trainers)
{
    int i;
    int j;

    //loops through each itinerary and frees its corresponding region array
    for(i = 0; i < num_trainers; i++)
    {
        free(itin[i].regions);
    }

    //frees the array of itinerary structs
    free(itin);
}

//frees trainer struct
void trainer_destructor(trainer *t, int num_trainers)
{
    int i;
    int j;

    //loops through each trainer and frees its corresponding name
    for(i = 0; i < num_trainers; i++)
    {
        free(t[i].name);
    }

    //frees the array of trainer structs
    free(t);
}



int main()
{
    atexit(report_mem_leak);
    int num_monsters;
    int num_regions;
    int num_itineraries;
    int num_trainers;
    FILE *ifp;
    FILE *ofp;
    ifp = fopen("as1-sample-input-2.txt", "r");
    ofp = fopen("output.txt", "w");
    monster *m;
    region *r;
    itinerary *itin;
    trainer *t;

    //gets the number of monsters
    //allocates space for the array of monster structs
    //calls monster constructor to create the monster structs
    num_monsters = get_num_monsters(ifp);
    m = malloc(sizeof(monster) * num_monsters);
    monster_constructor(ifp, num_monsters, m);

    //gets the number of regions
    //allocates space for the array of region structs
    //calls region constrcutor to create the region structs
    num_regions = get_num_regions(ifp);
    r = malloc(sizeof(region) * num_regions);
    region_constructor(ifp, num_regions, num_monsters, r, m);

    //gets the number of trainers/ itineraries
    //allocates space for the trainer and itinerary structs
    //calls trainer constructor to create trainer structs
    //trainer constructor later calls read trainer which calls itinerary constructor 
    num_trainers = get_num_trainers(ifp);
    printf("num_trainers: %d", num_trainers);
    itin = malloc(sizeof(itinerary) * num_trainers);
    t = malloc(sizeof(trainer) * num_trainers);
    trainer_constructor(ifp, num_trainers, t, r, itin, num_regions);

    output_to_file(ofp, m, r, itin, t, num_trainers);

    monster_destructor(m, num_monsters);
    region_destructor(r, num_regions);
    itinerary_destructor(itin, num_trainers);
    trainer_destructor(t, num_trainers);


    fclose(ifp);

    return 0;
}