#ifndef STATISTICS_H
#define STATISTICS_H

/* statistiche di funzionamento di un singolo task, all'interno di un intervallo di execuzione */
typedef struct
{
	unsigned task_id;    /* id del task a cui questa statistica è associato */
	unsigned cycle_id;   /* id (progressivo) dell'iperperiodo in cui questa statistica è stata calcolata */
	
	unsigned exec_count; /* numero di rilasci (esecuzioni) del task, andati a buon fine */
	unsigned miss_count; /* numero di deadline miss avvenute nell'esecuzione del task */
	unsigned canc_count; /* numero di mancate esecuzioni (cancellazioni) del rilascio del task */
	
	float avg_exec_time; /* tempo medio di esecuzione del task (millisecondi) */
	float max_exec_time; /* tempo massimo di esecuzione del task (millisecondi) */
	
} task_stats;

/* statistiche di funzionamento globali dell'applicazione */
typedef struct
{
	unsigned cycle_count; /* totale di iperperiodi eseguiti, dall'inizio dell'esecuzione */
	unsigned exec_count;  /* totale di rilasci (esecuzioni) dei task, andati a buon fine, dall'inizio dell'esecuzione  */
	unsigned miss_count;  /* totale di deadline miss avvenute nell'esecuzione dei task, dall'inizio dell'esecuzione */
	unsigned canc_count;  /* totale di mancate esecuzioni (cancellazioni) del rilascio dei task, dall'inizio dell'esecuzione */
} global_stats;

#endif
