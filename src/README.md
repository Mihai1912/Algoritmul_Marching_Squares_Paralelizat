Algoritmul Marching Squares Paralelizat
Descriere

Acest proiect conține o implementare a algoritmului Marching Squares paralelizat în limbajul C. 
Algoritmul este utilizat pentru a genera imagini de contur pe baza imaginilor de intrare și a 
unei valori de prag. Procesul de generare a contururilor este paralelizat folosind biblioteca 
pthread.

Funcționalități

Proiectul conține următoarele funcționalități cheie:

    Redimensionare Imagine: Codul poate redimensiona imaginea de intrare pentru a se potrivi 
    unei dimensiuni predefinite (RESCALE_X și RESCALE_Y). Aceasta simplifică procesul de 
    generare a conturului.

    Eșantionare Grilă: Imaginea redimensionată este împărțită într-o grilă de celule, iar 
    valoarea fiecărei celule este stabilită prin comparație cu valoarea de prag (sigma).

    Marching Squares: Acest pas analizează grila eșantionată pentru a identifica tipul de 
    contur pentru fiecare subgrilă și înlocuiește pixelii din imaginea originală cu pixelii 
    din imaginea de contur corespunzătoare.

    Paralelizare: Codul utilizează mai multe fire de execuție pentru a accelera procesul de 
    generare a contururilor. Fiecare fir de execuție gestionează o parte a imaginii de 
    intrare și efectuează operațiile de redimensionare, eșantionare și Marching Squares 
    independent.

Detalii despre Paralelizare

Codul utilizează biblioteca pthread pentru a realiza paralelizarea. Iată modul în care 
paralelizarea este aplicată algoritmului:

    Inițializare Fire de Execuție: Se creează și se inițializează un număr specificat 
    de fire de execuție (P) cu ajutorul bibliotecii pthread. Fiecare fir de execuție 
    va gestiona o parte a imaginii.

    Funcția Firului de Execuție (f): Fiecare fir de execuție execută aceeași funcție 
    (f) care efectuează următoarele etape:

        Redimensionarea Imaginei: Dacă este necesar, firul de execuție redimensionează 
        o parte a imaginii. Operația de redimensionare este paralelizată între firele de 
        execuție.

        Eșantionarea Grilei: Firul de execuție eșantionează o parte a imaginii și generează 
        o rețea de valori binare bazate pe pragul sigma. Acest proces este de asemenea 
        paralelizat.

        Marching Squares: Firul de execuție analizează rețeaua, identifică tipurile de 
        contur și înlocuiește pixelii din imaginea originală cu pixelii din imaginea de 
        contur corespunzătoare. Fiecare fir de execuție lucrează independent pe propria 
        parte a rețelei.

    Sincronizare cu Bariera: Se utilizează o barieră pthread pentru a sincroniza firele 
    de execuție în punctele critice. Aceasta asigură că toate firele de execuție completează 
    operațiile de redimensionare și eșantionare a grilei înainte de a trece la etapa 
    Marching Squares.

