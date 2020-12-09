DATA_DIR=$(pwd)/videos
MODELS_DIR=$(pwd)/models

if [ "$#" -eq 0 ]; then
    THREADS=1
else
    THREADS=$1
fi

if [ $THREADS -eq 1 ]; then
    MODE="-it"
else
    MODE="-d"
fi

for (( i = 0; i < $THREADS; i++ )) 
do
	echo "Launching thread number $(( $i + 1 )) of $THREADS"
	docker run --rm $MODE -v $(pwd):/workspace -v $DATA_DIR:/data -v $MODELS_DIR:/models --gpus all trimmer ./build/trimmer config/trimmer_config.json $i $THREADS
done


