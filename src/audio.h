typedef struct audio_data
{
    int left;
    int right;
    audio_data* next;
} audio_data;

audio_data current;

void push(int, int);