import datetime
import os

from threading import Lock

write_lock = Lock()


class CsvLogger:

    log_filename = "log.csv"

    @classmethod
    def log(cls, action, log_text):

        with write_lock:

            write_header = not os.path.exists(cls.log_filename)

            now = datetime.datetime.now()

            with open(cls.log_filename, "a") as file:

                if write_header:
                    file.write("datetime, action, log\n")

                file.write(f"{str(now)}, {action}, {log_text}\n")
