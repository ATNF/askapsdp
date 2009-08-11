module askap {
    module cp {
        module frontend {

            struct Metadata {
                long bat;
            };

            interface IMetadataStream {
                void handle(Metadata data);
            };

        };
    };
};
