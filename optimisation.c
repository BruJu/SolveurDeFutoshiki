
void trier_contraintes(Slot * slot) {
#define trier_contrainte_lautre_id(cont, me)	(((cont)->caseA == (me)) ? (cont)->caseB->id : (cont)->caseA->id)

	// Tri par insertion
	
	Contrainte * contrainte;
	int i, j;
	int min, min_v, cur_v;
	
	for (i = 0 ; i < slot->maxcontraintes ; i++) {
		
		min = i;
		min_v = trier_contrainte_lautre_id(slot->contraintes[i], slot);
		for (j = i + 1 ; i < slot->maxcontraintes ; j++) {
			cur_v = trier_contrainte_lautre_id(slot->contraintes[j], slot);
			
			if (min_v > cur_v) {
				min   = j;
				min_v = cur_v;
			}
		}
		
		if (min != i) {
			contrainte = slot->contraintes[i];
			slot->contraintes[i] = slot->contraintes[min];
			slot->contraintes[min] = contrainte;
		}
	}
	
}

