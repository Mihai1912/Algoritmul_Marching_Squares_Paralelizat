# Algoritmul Marching Squares Paralelizat

## Descriere

Scopul proiectului este de a crea o implementare mai eficienta a algoritmului Marching Squares
prin utilizarea bibliotecii pthread.

## Funcționalități

### Proiectul conține următoarele funcționalități cheie:

    1. Redimensionare Imagine:
    Codul poate redimensiona imaginea de intrare pentru a se potrivi 
    unei dimensiuni predefinite (RESCALE_X și RESCALE_Y). Aceasta simplifică procesul de 
    generare a conturului.

    2. Crearea Grid ului:
    Imaginea redimensionată este împărțită într-o grilă de celule, iar 
    valoarea fiecărei celule este stabilită prin comparație cu valoarea de prag (sigma).

    3. Marching Squares:
    Acest pas analizează grila eșantionată pentru a identifica tipul de 
    contur pentru fiecare subgrilă și înlocuiește pixelii din imaginea originală cu pixelii 
    din imaginea de contur corespunzătoare.

    4. Paralelizare:
    Codul utilizează mai multe fire de execuție pentru a accelera procesul de 
    generare a contururilor. Fiecare fir de execuție gestionează o parte a imaginii de 
    intrare și efectuează operațiile de redimensionare, crearea grid ului și Marching Squares 
    independent.

## Detalii despre Paralelizare

### Codul utilizează biblioteca pthread pentru a realiza paralelizarea. Iată modul în care paralelizarea este aplicată algoritmului:

    Inițializare Fire de Execuție: Se creează și se inițializează un număr specificat 
    de fire de execuție (P) cu ajutorul bibliotecii pthread. Fiecare fir de execuție 
    va gestiona o parte a imaginii.

    Functia f : Fiecare fir de execuție execută aceeași funcție (f) care efectuează următoarele etape:

        Redimensionarea Imaginei: Dacă este necesar, firul de execuție redimensionează 
        o parte a imaginii. Operația de redimensionare este paralelizată între firele de 
        execuție impartind numarul de linii din matrice(imagine) la numarul de threaduri.

        Crearea grid ului : Firul de execuție eșantionează o parte a imaginii și generează 
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

