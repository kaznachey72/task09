# Задание
Написать демон получения размера заданного через конфигурацию файла. Демон должен отдавать
текущий размер файла по запросу через сокет домена UNIX, заданный в конфигурации, и сразу разрывать
соединение с клиентом.

# Сложность
★★★☆☆

# Цель задания
Получить навыки создания демонов и взаимодействия через сокеты домена UNIX.

# Критерии успеха
1. Созданное приложение успешно запускается как в фоне, так и без демонизации.
2. Демон корректно отдаёт размер заданного файла до и после изменения файла.
3. Демон корректно обрабатывает ошибки доступа к файлу, в том числе ситуацию удаления заданного файла.
4. Код компилируется без предупреждений с ключами компилятора -Wall -Wextra -Wpedantic -std=c11.
5. Далее успешность определяется ревью кода.

# Обратная связь
Cтудент коммитит все необходимое в свой github/gitlab репозиторий. Далее необходимо зайти в ЛК, найти
занятие, ДЗ по которому выполнялось, нажать “Чат с преподавателем” и отправить ссылку. После этого
ревью и общение на тему ДЗ будет происходить в рамках этого чата.
