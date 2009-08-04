module askap {
    module cp {
        module cpfrontend {

            struct Metadata {
                long bat;
            };

            interface IMetadataStream {
                void handle(Metadata data);
            };

        };
    };
};
