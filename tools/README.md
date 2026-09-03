# Wykresy pokrycia tagów RFID

Skrypt `plot_tag_coverage.py` tworzy:

- wykres słupkowy liczby anten, które wykryły tag w każdej próbie, osobno
  dla półek 50–69, 70–89 oraz 90–109,
- mapę wykryć, na której każdy wiersz odpowiada konkretnej antenie,
  a każda kolumna konkretnemu tagowi,
- osobny wykres dla każdej anteny pokazujący wartości `1` lub `0` dla
  wszystkich tagów na każdej półce.

Pomarańczowe pole na mapie oznacza, że antena wykryła tag w każdej próbie
całego testu. Szare pole oznacza brak odczytu w co najmniej jednej próbie.
Tag nieobecny w CSV jest uwzględniany z wartością `0`.

Jeżeli dla taga `antenna_count` wynosi `0`, jego kolumna otrzymuje dodatkowo
czerwone, kreskowane tło, a numer taga na osi jest czerwony i pogrubiony.
Oznacza to, że żadna antena nie wykryła go niezawodnie we wszystkich próbach.

Skrypt nie otwiera portu COM i nie koliduje z Serial Monitorem PlatformIO.

## Konfiguracja półek

Na początku skryptu znajdują się ustawienia:

```python
FIRST_TAG_NUMBER = 50
TAGS_PER_SHELF = 20
SHELF_COUNT = 3
```

Obecna konfiguracja obejmuje 60 opakowań i tagi 50–109. Po dodaniu czwartej
półki wystarczy ustawić `SHELF_COUNT = 4`; wszystkie wykresy automatycznie
rozszerzą się wtedy do 80 opakowań i zakresu 50–129.

## Instalacja

W katalogu głównym projektu:

```powershell
python -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r tools\requirements.txt
```

## Użycie

Otwórz `tools\plot_tag_coverage.py` i wklej tabelę z Serial Monitora do
zmiennej `CSV_DATA` na początku pliku:

```python
CSV_DATA = """
epc,antenna_count,A1,A2,A5
000000000000000000000050,2,1,0,1
000000000000000000000051,1,0,1,0
000000000000000000000070,3,1,1,1
000000000000000000000090,2,1,1,0
"""
```

Kolumny `A1`, `A2`, `A5` odpowiadają antenom wykrytym przez ESP32.
Wartość `1` oznacza wykrycie EPC przez antenę we wszystkich próbach.
Jeżeli antena pominęła tag choć raz, otrzymuje `0`. `antenna_count` jest
sumą wartości `1` w danym wierszu.

Po zmianie kryterium trzeba wkleić CSV z nowego testu. Dane utworzone przez
starszy firmware oznaczały wykrycie przynajmniej raz i nie nadają się do
oceny niezawodności.

Możesz również wkleić cały fragment razem z liniami `TAG_CSV_BEGIN` oraz
`TAG_CSV_END` — zostaną automatycznie pominięte.

Następnie uruchom:

```powershell
.\.venv\Scripts\python.exe tools\plot_tag_coverage.py
```

Wykresy otworzą się w oknach i zostaną zapisane jako:

- `tools\output\tag_coverage.png`,
- `tools\output\antenna_tag_matrix.png`,
- `tools\output\antennas\antenna_A01_tags.png` itd.

Jeżeli nie chcesz otwierać okien, ustaw `SHOW_CHART = False` na początku
skryptu.

Wklejone wcześniej dane w starym, dwukolumnowym formacie nadal wygenerują
wykres słupkowy, ale nie zawierają informacji potrzebnych do utworzenia mapy
konkretnych anten.
