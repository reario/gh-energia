Create VIEW ss AS
       select
       secondi.data AS data,
       secondi.ora AS ora,
       secondi.autoc AS secondi_autoclave,
       scatti.autoc AS scatti_autoclave,
       secondi.pozzo AS second_pozzo,
       scatti.pozzo AS scatti_pozzo
       FROM secondi ,scatti  WHERE data_ora_secondi = data_ora_scatti;
