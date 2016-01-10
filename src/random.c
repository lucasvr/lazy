/*
 * ---------------------------------------------------------------------
 *  Lazy - play a CD and print actual track informations into stdout
 * ---------------------------------------------------------------------
 *
 * File: random.c
 *
 * Author: Ulysses Almeida <munky@maluco.com.br>
 * 
 * ---------------------------------------------------------------------
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * ---------------------------------------------------------------------
 */

#include <stdlib.h>
#include <time.h>
#include "random.h"

/*
 * randomize - generate the random seed by using the actual time in seconds
 */
void randomize(void)
{
    time_t sec;

    /* generate the seed */
    time(&sec);
    srand((int) sec);
}


/*
 * Random - receive the mininum and maximum track numbers acceptable, and return the integer 
 * 			generated by the rand() operation
 */
int Random(min, max)
int min, max;
{
    int num;

    /* generate a random number */
    num = min + (int) ((float) max * rand() / (RAND_MAX + 1.0));

    /* return it */
    return (num);
}