Ayabaktî Melih-Muhammed
Gheorghe Andrei

Echipa #01, 413E

# **Documentație Mini Proiect – Pedală Digitală<sup>[\[1\]](#footnote-2)</sup>**

**Demonstrație VIDEO:**

[**https://drive.google.com/file/d/1yh_6hRmMxYOT_8DL-2sl9Pj-EA105Yl2/view?usp=sharing**](https://drive.google.com/file/d/1yh_6hRmMxYOT_8DL-2sl9Pj-EA105Yl2/view?usp=sharing)

1.  **Ghid de utilizare:**

**Obiecte necesare:**

- O sursă de semnal sonor de tensiune joasă (Vpp <= 1V). Ex: Chitară, Bass, orice instrument electric fără preamplificare;
- Un difuzor / amplificator;
- Un laptop.

**Mod de conectare:**

- Se conectează cabluri de 3.5mm de la sursa de semnal la jack-ul INPUT și de la jack-ul OUTPUT la difuzor/amplificator;
- Se conectează modulul ESP32 la un laptop

**Mod de utilizare:**

- Pe laptop, se accesează seriala UART a modului ESP32, de acolo, se pot introduce o serie de comenzi care activează/dezactivează anumite efecte sonore.

|     |     |     |     |     |
| --- | --- | --- | --- | --- |
| **Comanda** | **Efect** | **Parametru 1** | **Parametru 2** | **Parametru 3** |
| **OFF** | Oprirea tuturor efectelor | \-  | \-  | \-  |
| **HDIST** | Distors cu hard clipping | GAIN | \-  | \-  |
| **SDIST** | Distors cu soft clipping | GAIN | \-  | \-  |
| **CHORUS** | Efect de cor | RATE | DEPTH | MIX |
| **DELAY** | Delay / Ecou (la delay mare) | FEEDBACK | MIX | TIMP DELAY |
| **TREM** | Tremolo | RATE | DEPTH | MIX |
| **PHASER** | Phaser | RATE | DEPTH | \-  |

**Semnificația efectelor si parametrilor:**

- Efectul de distors este de fapt o amplificare a semnalului până când capetele semnalului întrec limitele posibile la ieșire. Hard clipping înseamnă că la limita de sus și de jos este tăiat brusc, iar soft clipping înseamnă că există un grad de amortizare a tăierii.
- Efectul de cor (Chorus) este implementat prin suprapunerea semnalului cu o copie a semnalului care este luată din spate cu un delay mic care variază în timp. Timpul de delay este decis de un LFO (Low Frequency Oscilator). RATE este frecventa LFO-ului, DEPTH este amplitudinea LFO-ului iar MIX este un parametru de la 0 la 1 care reprezintă rata amestecului celor două semnale (0 – semnal curat, 1 – doar semnalul cu delay).
- Efectul Delay seamănă cu Chorus, doar că timpul de delay este constant în loc de variabil și poate să fie mult mai mare. FEEDBACK reprezintă viteza cu care dispare delay-ul, iar TIMP DELAY este numărul de sample-uri cu care se uită în spate. Daca alegem un timp mare (>>5000 sample-uri), atunci efectul devine ceea ce numim Ecou.
- Tremolo seamănă și el cu Chorus, prin faptul că și acest efect este și el controlat de un LFO. Însă, în loc de un delay, LFO-ul modulează amplitudinea semnalului de ieșire. RATE este frecvența, DEPTH reprezintă cât de mult scade amplitudinea semnalului atunci când LFO-ul își atinge valoarea maximă (0 – Semnalul nu e modulat deloc, 1 – Amplitudinea semnalului e modulată până la 0).
- Efectul Phaser este un efect de modulatie clasic, care imparte sunetul original in 2, modificand faza unuia dintre semnale. Si aici LFO moduleaza amplitudinea semnalului. RATE este frecventa, iar DEPTH reprezinta „intensitatea”, „adancimea” efectului

2.  **Conexiuni și schematica dispozitivului:**

Pentru ca microcontrolerul să poată citi corect semnalul de intrare, acesta are nevoie de un semnal între 0-1.1V<sup>[\[2\]](#footnote-3)</sup>. Semnalul de intrare este însă un semnal slab, care este centrat în 0 (adică are și valori negative) și are impedanță mare. Pentru a putea fi citit, trebuie mai întâi centrat într-o valoare pozitivă (în cazul nostru 0.55V) iar apoi trecut printr-un buffer/amplificator fără inversiune. Deoarece tensiunea maximă poate varia de la semnal la semnal, amplificarea poate fi reglată printr-un potențiometru (Factorul de amplificare variază de la 2 la 4). La ieșire din ESP, trebuie eliminată componenta continuă a semnalului, așa că acesta trece printr-un condensator, iar apoi printr-un filtru trece jos pentru eliminarea zgomotului de fundal adăugat de factori precum zgomotul sursei de alimentare și bruiajul cauzat de cablaj.


<img width="1058" height="793" alt="image" src="https://github.com/user-attachments/assets/2af9edd4-94cb-4044-a0fe-b22d44156d78" />
_(Poză cu dispozitivul și semnificația fiecarei părți)_


<img width="1144" height="617" alt="image" src="https://github.com/user-attachments/assets/7232d847-a31e-44b6-8c95-9dd9877b1661" />
_(Schematica circuitului)_

3.  **Librarii Utilizate**

- **driver/adc.h –** Libraria folosita pentru a accesa convertorul analog-digital.
- **driver/dac.h** – Libraria folosita pentru a accesa convertorul digital-analog.
- **String.h** – Libraria folosita pentru a procesa datele de intrare din seriala UART.

4.  **Bibliografie:**

- https://electronics.howstuffworks.com/gadgets/audio-music/guitar-pedal.htm
- https://youtu.be/OSCKBmkrH_g?si=LobbcDQZIldl9G8D
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc/index.html
- https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/dac.html

1.  Termenul „pedală” a căpătat, în timp, un sens mai larg, acesta nu se referă neapărat la un dispozitiv al cărui mod de operare este analog cu o pedală de picior, ci se referă la orice dispozitiv compact folosit pentru procesarea semnalului de la instrumente electrice care se află conectat între instrument și difuzor/amplificator. [↑](#footnote-ref-2)
    
2.  Prin atenuare, un ESP32 poate măsura și până la 3.3V, doar ca natura Amplificatorilor Operaționali cauzează semnalul care iese din aceștia să aibă maximul la **Vcc– 1.5V**, unde **Vcc** este tensiunea de alimentare, asta înseamnă în cazul nostru o tensiune maximă de 1.7V, ceea ce înseamnă că nu putem centra semnalul la jumătatea intervalului 0-3.3V, adică 1.65V.
    
    [↑](#footnote-ref-3)
