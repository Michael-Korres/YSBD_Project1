Project 1 – YSBD | ΚΟΝΤΟΣ ΑΓΓΕΛΟΣ: 11115202000089 & ΚΟΡΡΕΣ ΜΙΧΑΗΛ: 11115202000092
ASK. 1  
HP
    • HP_CreateFile
        Δημιουργεί ένα κενό αρχείο σωρού.
        Αυτό σημαίνει,ότι θα έχει ένα block,το οποίο θα έχει ειδικές πληροφορίες.Άρα:
        • Δημιουργώ το αρχείο
        • Το ανοίγω
        • Δημιουργώ μπλοκ στη μνήμη και το δεσμεύω για το αρχείο
        • Αποθηκεύω την ειδική πληροφορία σε μια βολική δομή(HP_info)(ώστε με παρόμοιο τρόπο να την αντλήσω μετά στην HP_OpenFile).
            (Δε χρειάζεται Memcpy σε αυτό το σημείο!)
            Η μνήμη υπάρχει δεσμευμένη στο block και απλώς με την ειδική δομή,αποθηκεύουμε δομημένα την πληροφορία στο ειδικό (1ο) block.
            Υπολογίζω και τη χωρητικότητα των block για εγγραφές.
        • Κάνω dirty και unpinned το block
        • Τα κλείνω όλα και κομπλέ η CREATE.


        Από το παραπάνω καταλαβαίνουμε πως δεν υπάρχει block_index(ευρετήριο για εγγραφές στο μπλοκ),αλλά οι εγγραφές είναι στην αρχή του block ώστε να γίνεται άμεση προσπέλαση.Ακόμη, το header του μπλοκ που περιλαμβάνει την πληροφορία του βρίσκεται στο τέλος του.
        • Επίσης,μπορώ να υπολογίσω το records/block ως:    (Μέγεθος block  - Μέγεθος Κεφαλής του block)/μέγεθος record

    • HP_OpenFile
        • Με όμοιο τρόπο αντλώ την ειδική πληροφορία του συγκεκριμένου αρχείο και θέτω τον describer του στην HP_info
        • Όπως σημειώνεται και κάτω-κάτω το 1ο block γίνεται unpinned(και συνεπώς destroyed) στο κλείσιμο του αρχείου
 

    • HP_CloseFile
        • Κάνει unpinn το 1ο block και κλείνει το αρχείο
 


    • HP_InsertEntry()
        • Υπάρχουν 3 ενδεχόμενα:
        1. Το αρχείο είναι κενό(έχει μόνο το ειδικό block).
            ▪ Άρα θα πρέπει να δημιουργήσω νέο μπλοκ και να εγγράψω το record στην αρχή του.
            ▪ Θα πρέπει επομένως, να δημιουργήσω HP_block_info(Κεφαλή του block) και να την εγγράψω στο τέλος του block .
            ▪ Τέλος, θα πρέπει να ενημερώσω την HP_info.
        2. Το αρχείο έχει block και χωράει τη νέα εγγραφή.
            ▪ Εγγράφω το record στο διαθέσιμο χώρο και να ανανεώνω την υπάρχουσα HP_block_info.
        3. Το αρχείο έχει block,αλλά δε χωράει τη νέα εγγραφή
            ▪ Άρα θα πρέπει να δημιουργήσω νέο μπλοκ και να εγγράψω το record στην αρχή του.
            ▪ Θα πρέπει επομένως,να δημιουργήσω HP_block_info(Κεφαλή του block) και να την εγγράψω στο τέλος του block.
            ▪ Τέλος,θα πρέπει να ενημερώσω την HP_info.
    • Στο μεταξύ θα πρέπει να γίνονται αντίστοιχες ενημερώσεις στο αρχικό block του αρχείου.
    • Στην υλοποίηση θα συμπτύξουμε τις περιπτώσεις 1. & 2. Θα αξιοποιήσουμε το εξής κόλπο: Θα αρχικοποιήσουμε τον αριθμό εγγραφών του 
      πρώτου block " >"(μεγαλύτερο) από το maximum capacity,οπότε ο κώδικας θα αντιμετωπίζει το 1ο μπλοκ σαν ένα οποιοδήποτε «γεμάτο» μπλοκ,
      άρα θα δημιουργεί ένα νέο για την εγγραφή.


    • HP_GetAllEntries()
        Υπάρχει πολύ καλό commenting για την επεξήγηση της συλλογιστικής πορείας.
        • Παίρνουμε κάθε μπλόκ του αρχείου ξεκινώντας από το 2ο.
        • Παίρνουμε κάθε εγγραφή στο μπλοκ.
        • Ελέγχουμε το κλειδί και αν είναι ίσο με value,στέλνουμε πίσω τον αριθμό των block που περάσαμε(χωρίς να μετράμε το 1ο)
        • Σε κάθε επανάληψη κάνουμε unpinn το block,καθώς δε το χρειαζόμαστε άλλο,undirty αφού μόνο διαβάσαμε το περιεχόμενό του.
        


ΣΗΜΑΝΤΙΚΕΣ ΠΑΡΑΤΗΡΗΣΕΙΣ:
    • Ξεκινάμε με init και στο τέλος κάνουμε destroy τα blocks.Το 1ο το κάνουμε init και το unpinned (αλλά και το destroy του) του γίνεται πριν το κλείσιμο του αρχείου διότι περιέχει την τοποθεσία όπου βρίσκεται η HP_info
    • Χρησιμοποιούμε MRU ώστε να φεύγουν γρήγορα τα πολλά blocks που δημιουργούνται στο HP_GetAllEntries().
    • Έχουν γίνει αλλαγές στις ονομασίες των ορισμάτων των συναρτήσεων,αλλά ΤΑ ΠΡΩΤΟΤΥΠΑ ΕΧΟΥΝ ΔΙΑΤΗΡΗΘΕΙ ΙΔΙΑ ΜΕ ΤΗΝ ΕΚΦΩΝΗΣΗ.
    • Υπάρχουν κάποια μέλη στον ορισμό των structs που δεν χρησιμοποιούνται.
    • Για testing χρησιμοποιήθηκαν οι hp_main.c (με την προσθήκη της time() στο srand()) και mock_main.c.

HT

• HT_CreateFile
    • Φτιάχνει ένα BF αρχείο όπου το 1ο block θα έχει όλες τις αρχικοποιήσεις(πληροφορίες για τα άλλα blocks) και ένα όνομα 'ΗΤ',αναγνωριστικό των αρχείων HT.
    • Η blocksNeeded μεταβλητή κρατάει πόσα block θα χρησιμοποιούνται σε κάθε bucket,ώστε κάθε bucket να έχει ίσο αριθμό blocks(κάποια blocks δε θα χρησιμοποιηθούν).
    • Μετά από το όνομα(HT) υπάρχει ένα array από ints με μήκος = num_of_blocks,όπου κάθε στοιχείο του περιέχει πόσα records υπάρχουν στο αντίστοιχο block,αν κάθε block έχει αριθμηθεί σε μια σειρά.

• HT_OpenFile
    • Έλεγχος τύπου αρχείου(μέσω του ονόματος)
    • Άντληση των πληροφοριών του αρχείου

• HT_InsertEntry
    • block number: Είναι το index του block (Αρίθμηση 0 για κάθε hash) στο αντίστοιχο hash όπου θα μπει η εγγραφή.
    • block_to_see: Είναι το 1ο block (που περιέχει τις πληροφορίες)
    • block_to_insert : Το block που βρίσκουμε(μέσω του block number)
    • Αν το block_to_insert είναι γεμάτο πηγαίνουμε στο επόμενο,μέχρι να βρούμε κάποιο με διαθέσιμο χώρο
    • curr_number: Παίρνει από το 1ο block το αντίστοιχο number του block που θέλουμε (το array ξεκινά 8 byte μετά την αρχή των data του 1ου block,άρα αρχή+8)

ASK. 2

SHT
    • Όμοια λογική,με επιπλέον,ένα πίνακα ονομάτων που συνενώνεται με τον πίνακα των ids.