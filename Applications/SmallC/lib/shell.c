/* Shell sort of string v[0] .... v[n-1] into increasing
 * order.
 *      Reference CPL pg. 108.
 */

shellsort(v, n)
int v[];
int n;
        {
        int gap, i, j;
        char *temp;
        for (gap = n/2; gap > 0; gap = gap / 2)
                for (i = gap; i < n; i++)
                        for (j = i - gap; j >= 0; j = j - gap){
                                if (strcmp(v[j], v[j+gap]) <= 0)
                                        break;
                                temp = v[j];
                                v[j] = v[j + gap];
                                v[j + gap] = temp;
                                }
        }
