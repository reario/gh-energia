#!/bin/bash

psql -U reario reario_db < /home/reario/energia/postgres/report-events.sql | ./sendEmail.pl -f GH@giannini.homeip.net -t vittorio.giannini@mail.wind.it -u report
#psql -U reario events < /home/reario/energia/postgres/report-events.sql |/sendEmail.pl -f GH@giannini.homeip.net -t vittorio.giannini@inwind.it -u report
