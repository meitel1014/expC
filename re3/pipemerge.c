/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This program demonstrates the use of the merge sort algorithm.  For
 * more information about this and other sorting algorithms, see
 * http://linux.wku.edu/~lamonml/kb.html
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_ITEMS 10

void mergeSort(int numbers[], int temp[], int array_size);
void m_sort(int numbers[], int temp[], int left, int right);
void merge(int numbers[], int temp[], int left, int mid, int right);

int numbers[NUM_ITEMS];
int temp[NUM_ITEMS];

int main(){
	int pid, status, fd[2], buf[NUM_ITEMS / 2], check = -1;

	//seed random number generator
	srand(getpid());

	//fill array with random integers
	for(int i = 0; i < NUM_ITEMS; i++)
		numbers[i] = rand();

	if(pipe(fd) == -1){
		perror("pipe");
		exit(1);
	}
	if((pid = fork()) == -1){
		perror("fork");
		exit(1);
	}
	if(pid == 0){ /* Child process */
		close(fd[0]);

		mergeSort(numbers, temp, NUM_ITEMS / 2);

		if(write(fd[1], numbers, NUM_ITEMS / 2 * sizeof(int)) == -1){
			perror("pipe write");
			exit(1);
		}
		exit(0);
	}else{ /* Parent process */
		close(fd[1]);

		mergeSort(numbers + NUM_ITEMS / 2, temp, NUM_ITEMS - NUM_ITEMS / 2);

		if(read(fd[0], buf, NUM_ITEMS / 2 * sizeof(int)) == -1){
			perror("pipe read");
			exit(1);
		}
		memcpy(numbers, buf, NUM_ITEMS / 2 * sizeof(int));
		wait(&status);
	}

	merge(numbers, temp, 0, NUM_ITEMS / 2, NUM_ITEMS - 1);

	printf("Done with sort.\n");

	for(int i = 0; i < NUM_ITEMS; i++){
		if(numbers[i] < check){
			printf("error");
		}
		check = numbers[i];
		printf("%i\n", numbers[i]);
	}
	return 0;
}

void mergeSort(int numbers[], int temp[], int array_size){
	m_sort(numbers, temp, 0, array_size - 1);
}

void m_sort(int numbers[], int temp[], int left, int right){
	int mid;

	if(right > left){
		mid = (right + left) / 2;
		m_sort(numbers, temp, left, mid);
		m_sort(numbers, temp, mid + 1, right);

		merge(numbers, temp, left, mid + 1, right);
	}
}

void merge(int numbers[], int temp[], int left, int mid, int right){
	int i, left_end, num_elements, tmp_pos;

	left_end = mid - 1;
	tmp_pos = left;
	num_elements = right - left + 1;

	while((left <= left_end) && (mid <= right)){
		if(numbers[left] <= numbers[mid]){
			temp[tmp_pos] = numbers[left];
			tmp_pos = tmp_pos + 1;
			left = left + 1;
		}else{
			temp[tmp_pos] = numbers[mid];
			tmp_pos = tmp_pos + 1;
			mid = mid + 1;
		}
	}

	while(left <= left_end){
		temp[tmp_pos] = numbers[left];
		left = left + 1;
		tmp_pos = tmp_pos + 1;
	}
	while(mid <= right){
		temp[tmp_pos] = numbers[mid];
		mid = mid + 1;
		tmp_pos = tmp_pos + 1;
	}

	for(i = 0; i <= num_elements; i++){
		numbers[right] = temp[right];
		right = right - 1;
	}
}
