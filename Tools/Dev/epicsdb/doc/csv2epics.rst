csv2epics
=========

usage::

    csv2epics.py --prefix="pvPrefix" --input="dgs.csv" --output="epics.db"

         --prefix     - PV name prefix to apply to all PV names
         --input      - input csv file (can specify multiple times)
         --patch      - patch csv file for existing records (can specify multiple times)
         --output     - output EPICS db file to save to
         --no-comment - supress comments is output db
