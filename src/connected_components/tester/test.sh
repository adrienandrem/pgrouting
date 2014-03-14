# sudo -i -u postgres -H psql -c "ALTER ROLE user WITH SUPERUSER ;"
dropdb pgr_dev
createdb pgr_dev
psql pgr_dev -c "CREATE EXTENSION postgis ;"
psql pgr_dev -c "CREATE EXTENSION pgrouting ;"
