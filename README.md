### INSTRUKCJA KOMPILACJI
W celu skompilowania plików należy uruchomić plik `compile.sh`; wygeneruje on dwa pliki: `server.out` oraz `client.out`

### INSTRUKCJA URUCHOMIENIA
Należy najpierw jednokrotnie uruchomić plik `server.out`, a dopiero potem wybraną ilość programów `client.out`.

Aby użytkownik mógł się zalogować, w polu `Server id` musi podać ciąg `1234` zatwierdzony enterem.

Utworzonych jest 9 użytkowników:

|login|hasło|
|---|---|
|test1|1|
|test2|2|
|test3|3|
|test4|4|
|test5|5|
|test6|6|
|test7|7|
|test8|8|
|test9|9|

Korzystanie z programu polega na wyborze cyfry intsrukcji.

Aby wysłać wiadomość do użytkownika należy wprowadzić jego pełną nazwę, przykładowo `test3`.

Aby wysłać wiadomość do grupy użytkowników należy wprowadzić jedynie jej indeks, przykładowo, w celu wysłania wiadomości do grupy `group0` należy podać `0`. 

Dołączanie i opuszczanie grup przebiega w sposób analogiczny.


### Opis plików

* inf151918_s.c:
Plik zawiera kod serwera, niezbędne jest jego uruchomienie jako pierwszego ponieważ on tworzy kolejkę komunikatów. Przetwarza całą logikę przesyłania wiadomości. Została w nim zaimplementowana tablica mieszająca, zawierająca wszystkich użytkowników.

* inf151918_k.c:
Plik zawiera kod klienta, nie zawiera żadnych funkcji przetwarzających logikę przesyłania wiadomości, wszystko to dzieje się po stronie serwera.  


